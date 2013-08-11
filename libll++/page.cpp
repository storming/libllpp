#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#include "page.h"
#include "etc.h"


namespace ll {

const unsigned page_allocator::sys_page_size = sysconf(_SC_PAGESIZE);
page_allocator *page_allocator::_global = nullptr;

page_allocator::page_allocator() : _iseg(), _cseg(), _pages(), _areas(), _segments()
{
    memset(_freetab, 0, sizeof(_freetab));
}

page_allocator::~page_allocator() 
{
    segment  *seg;
    while ((seg = _segments.pop_front())) {
        segment_destroy(seg);
    }
}

page_allocator::segment *page_allocator::segment_create(page_allocator::segment *seg, unsigned size) 
{
    size = ll_align(size, sys_page_size);

    char *p = (char*)mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        memory_fail();
    }

    if (!seg) {
        seg = (segment*)p;
        seg->_firstp = p + sizeof(segment);
    }
    else {
        seg->_firstp = p;
    }
    seg->_base = p;
    seg->_endp = p + size;
    _segments.push_front(seg);
    return seg;
}

void page_allocator::segment_destroy(page_allocator::segment *seg)
{
    munmap(seg->_base, seg->_endp - seg->_base);
}

bool page_allocator::segment_expand(page_allocator::segment *seg, unsigned size)
{
    unsigned old_size = seg->_endp - seg->_base;
    unsigned new_size = old_size + ll_align(size, sys_page_size);

    char *p = (char*)mremap(seg->_base, old_size, new_size, 0);
    if (p == MAP_FAILED) {
        return false;
    }
    assert(p == seg->_base);
    seg->_endp = p + new_size;
    return true;
}

inline void *page_allocator::ialloc(unsigned size) 
{
    char *p;
    size = ll_align_default(size);

    while (1) {
        if (!_iseg) {
            _iseg = segment_create(NULL, iseg_initsize);
        }

        if (ll_unlikely(_iseg->_firstp + size > _iseg->_endp)) {
            if (!segment_expand(_iseg, iseg_expand_size)) {
                _iseg = NULL;
                continue;
            }
        }

        p = _iseg->_firstp;
        _iseg->_firstp += size;
        return p;
    }
}

inline page_allocator::area *page_allocator::area_alloc(void *base)
{
    area *a = _areas.pop_front();
    if (!a) {
        a = (area*)ialloc(sizeof(area));
        unsigned i;
        page_node *node = a->_pages;
        for (i = 0; i < area::length; i++, node++) {
            node->_index = i;
        }
    }
    a->_base = (char*)base;
    a->_pages->_order = max_order;
    return a;
}

inline void page_allocator::area_free(page_allocator::area *a)
{
    chunk_free(a->_base);
    _areas.push_front(a);
}

inline page *page_allocator::page_alloc()
{
    page *pg = _pages.pop_front();
    if (!pg) {
        pg = (page*)ialloc(sizeof(page));
    }
    return pg;
}

inline void page_allocator::page_free(page *pg)
{
    _pages.push_front(pg);
}

inline void *page_allocator::chunk_alloc()
{
    void *p = (void*)_freetab->pop_front();
    if (p) {
        return p;
    }

    while (1) {
        if (!_cseg) {
            _cseg = (segment*)ialloc(sizeof(segment));
            segment_create(_cseg, cseg_initsize);
        }

        if (ll_unlikely(_cseg->_firstp + page_max_size > _cseg->_endp)) {
            if (!segment_expand(_cseg, cseg_expand_size)) {
                _cseg = NULL;
                continue;
            }
        }

        p = _cseg->_firstp;
        _cseg->_firstp += page_max_size;
        return p;
    }
}

inline void page_allocator::chunk_free(void *chunk)
{
    _freetab->push_front((page_node*)chunk);
}

page *page_allocator::alloc(size_t size)
{
    unsigned order, norder;
    page_node *node, *buddy;
    area *a;

    page *pg = page_alloc();

    size = ll_align(size, sys_page_size);

    if (ll_likely(size <= (page_max_size >> 1))) {
        size >>= page_allocator::page_boundary_index;
        order = bitorder::ordertab[size];
        if (order < min_order) {
            order = min_order;
        }

        freelist_t *list = _freetab + order;
        if ((node = list->pop_front())) {
buddy_final:
            node->_order = 0;
            pg->order_size = order;
            pg->base = node;
            a = (area*)(node - node->_index);
            pg->firstp = a->_base + (node->_index * page_min_size);
            pg->endp = pg->firstp + (1 << (page_boundary_index + order));
            return pg;
        }

        norder = order + 1;
        for (list++; norder < max_order; norder++, list++) {
            if ((node = list->pop_front())) {
                break;
            }
        }
        
        if (!node) {
            void *chunk = chunk_alloc();
            a = area_alloc(chunk);
            node = a->_pages;
            norder = max_order;
        }

        unsigned offset = 1 << (norder - min_order);
        while (norder > order) {
            norder--;
            offset >>= 1;
            buddy = node + offset;
            node->_order = buddy->_order = norder;
            _freetab[norder].push_front(buddy);
        }
        goto buddy_final;
    }
    else if (size == page_max_size) {
        pg->base = chunk_alloc();
        pg->firstp = (char*)pg->base;
        pg->endp = pg->firstp + page_max_size;
        pg->order_size = max_order;
        return pg;
    }
    else {
        pg->base = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        pg->order_size = size;
        pg->firstp = (char*)pg->base;
        pg->endp = pg->firstp + size;
        return pg;
    }
}

void page_allocator::free(page *pg) 
{
    if (ll_likely(pg->order_size < max_order)) {
        page_node *node = (page_node*)pg->base;
        page_node *buddy;
        unsigned buddy_index;
        unsigned order = pg->order_size;

        while (order < max_order) {
            buddy_index = node->_index ^ (1 << (order - min_order));
            buddy = node + (buddy_index - node->_index);
            if (order != buddy->_order) {
                node->_order = order;
                break;
            }

            freelist_t::remove(buddy);

            if (node > buddy) {
                node->_order = 0;
                node = buddy;
            }
            else {
                buddy->_order = 0;
            }

            order++;
            node->_order = order;
        }

        if (order == max_order) {
            area_free((area*)node);
        }
        else {
            _freetab[order].push_front(node);
        }

    }
    else if (pg->order_size == max_order) {
        chunk_free(pg->base);
    }
    else {
        munmap(pg->base, pg->order_size);
    }
    page_free(pg);
}


}; // namespace ll end

