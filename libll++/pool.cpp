#include <cassert>
#include <new>

#include "pool.h"
#include "module.h"

/* the following code refer to apr pool */

/* one-way circular list, can be removed self. */

/* Node list management helper macros; list_insert() inserts 'node'
 * before 'point'. */
#define list_insert(node, point) do {           \
    node->ref = point->ref;                     \
    *node->ref = node;                          \
    node->next = point;                         \
    point->ref = &node->next;                   \
} while (0)

/* list_remove() removes 'node' from its list. */
#define list_remove(node) do {                  \
    *node->ref = node->next;                    \
    node->next->ref = node->ref;                \
} while (0)

namespace ll {

pool *pool::_global = nullptr;

/* managed */
pool *pool::create(pool *parent, page_allocator *allocator)
{
    assert(parent);

    if (!allocator) {
        allocator = parent->_allocator;
    }

    pool *p;
    page *pg = allocator->alloc(page_allocator::page_min_size);
    pg->next = pg;
    pg->ref = &pg->next;

    p = (pool*)pg->firstp;
    p->_parent = parent;
    parent->_children.push_front(p);
    p->_allocator = allocator;
    p->_active = p->_self = pg;
    p->_children.init();
    pg->firstp = p->_firstp = (char *)p + ll_align_default(sizeof(pool));
    return p;
}

/* unmanaged */
pool *pool::create(page_allocator *allocator)
{
    assert(allocator);

    pool *p;
    page *pg = allocator->alloc(page_allocator::page_min_size);
    pg->next = pg;
    pg->ref = &pg->next;

    p = (pool*)pg->firstp;
    p->_parent = nullptr;
    p->_allocator = allocator;
    p->_active = p->_self = pg;
    p->_children.init();
    pg->firstp = p->_firstp = (char *)p + ll_align_default(sizeof(pool));
    return p;
}

void pool::destroy(pool *p)
{
    pool *child;
    while ((child = static_cast<pool*>(p->_children.front()))) {
        destroy(child);
    }

    if (p->_parent) {
        pool_list_t::remove(p);
    }

    page_allocator *a = p->_allocator;
    page *tmp;
    page *pg = p->_self;
    *pg->ref = nullptr;

    while (pg) {
        tmp = pg->next;
        a->free(pg);
        pg = tmp;
    }
}

void pool::clear()
{
    pool *child;
    while ((child = static_cast<pool*>(_children.front()))) {
        destroy(child);
    }


    page *tmp;
    page *pg = _active = _self;
    pg->firstp = _firstp;

    if (pg->next == _self) {
        return;
    }

    *pg->ref = nullptr;
    pg = pg->next;

    while (pg) {
        tmp = pg->next;
        _allocator->free(pg);
        pg = tmp;
    }

    _self->next = _self;
    _self->ref = &_self->next;
}

void *pool::alloc(page *active, size_t size)
{
    page *pg = active->next;

    if (size <= pg->space()) {
        list_remove(pg);
    }
    else {
        pg = _allocator->alloc(size);
    }

    pg->index = 0;

    void *p = pg->firstp;
    pg->firstp += size;

    list_insert(pg, active);
    _active = pg;

    unsigned index = (ll_align(active->space() + 1, page_allocator::page_boundary_size) - 
                      page_allocator::page_boundary_size) >> page_allocator::page_boundary_index;

    active->index = index;

    page *node = active->next;
    if (index >= node->index) {
        return p;
    }

    do {
        node = node->next;
    }
    while (index < node->index);

    list_remove(active);
    list_insert(active, node);

    return p;
}

char *pool::strdup(const char *str, size_t n)
{
    char *res;
    const char *end;

    if (str == NULL) {
        return NULL;
    }
    end = (const char*)memchr(str, '\0', n);
    if (end != NULL) {
        n = end - str;
    }
    res = (char*)alloc(n + 1);
    memcpy(res, str, n);
    res[n] = '\0';
    return res;
}

char *pool::strdup(const char *str)
{
    char *res;
    size_t len;

    if (str == NULL) {
        return NULL;
    }
    len = strlen(str) + 1;
    res = (char*)alloc(len);
    memcpy(res, str, len);
    return res;
}

struct pool_module {
    int module_init() {
        pool::_global = pool::create(page_allocator::global());
        return 0;
    }
};

ll_module(pool_module);
};
