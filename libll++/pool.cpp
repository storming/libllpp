#include <cassert>
#include <new>

#include "pool.h"
#include "module.h"
#include "printf.h"

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

#define SPRINTF_MIN_STRINGSIZE 32
struct pool_vbuff : public printf_formatter::buff {
    page *_node;
    pool *_owner;
    bool _got_a_new_node;
    page *_freelist;

    int flush() {
        page *node, *active;
        size_t cur_len, size;
        char *strp;
        size_t index;

        active = _node;
        strp = curpos;
        cur_len = strp - active->firstp;
        size = cur_len << 1;

        /* Make sure that we don't try to use a block that has less
         * than APR_PSPRINTF_MIN_STRINGSIZE bytes left in it.  This
         * also catches the case where size == 0, which would result
         * in reusing a block that can't even hold the NUL byte.
         */
        if (size < SPRINTF_MIN_STRINGSIZE)
            size = SPRINTF_MIN_STRINGSIZE;

        node = active->next;
        if (!_got_a_new_node && size <= node->space()) {

            list_remove(node);
            list_insert(node, active);

            node->index = 0;

            _owner->_active = node;

            index = (ll_align(active->space() + 1, page_allocator::page_boundary_size) - 
                     page_allocator::page_boundary_size) >> page_allocator::page_boundary_index;

            active->index = index;
            node = active->next;
            if (index < node->index) {
                do {
                    node = node->next;
                }
                while (index < node->index);

                list_remove(active);
                list_insert(active, node);
            }

            node = _owner->_active;
        }
        else {
            node = _owner->_allocator->alloc(size);

            if (_got_a_new_node) {
                active->next = _freelist;
                _freelist = active;
            }

            _got_a_new_node = true;
        }

        memcpy(node->firstp, active->firstp, cur_len);

        _node = node;
        curpos = node->firstp + cur_len;
        endpos = node->endp - 1; /* Save a byte for NUL terminator */

        return 0;
    }
};

char *pool::vsprintf(const char *fmt, va_list ap)
{
    pool_vbuff vb;
    char *strp;
    size_t size;
    page *active, *node;
    size_t index;

    vb._node = active = _active;
    vb._owner = this;
    vb.curpos = vb._node->firstp;

    /* Save a byte for the NUL terminator */
    vb.endpos = vb._node->endp - 1;
    vb._got_a_new_node = false;
    vb._freelist = nullptr;

    /* Make sure that the first node passed to apr_vformatter has at least
     * room to hold the NUL terminator.
     */
    if (vb._node->firstp == vb._node->endp) {
        vb.flush();
    }

    if (printf_formatter::format(&vb, fmt, ap) == -1) {
        return nullptr;
    }

    strp = vb.curpos;
    *strp++ = '\0';

    size = strp - vb._node->firstp;
    size = ll_align_default(size);
    strp = vb._node->firstp;
    vb._node->firstp += size;

    node = vb._freelist;
    page *tmp;
    while (node) {
        tmp = node->next;
        _allocator->free(node);
        node = tmp;
    }

    /*
     * Link the node in if it's a new one
     */
    if (!vb._got_a_new_node) {
        return strp;
    }

    active = _active;
    node = vb._node;

    node->index = 0;

    list_insert(node, active);

    _active = node;

    index = (ll_align(active->space() + 1, page_allocator::page_boundary_size) - 
             page_allocator::page_boundary_size) >> page_allocator::page_boundary_index;

    active->index = index;
    node = active->next;

    if (index >= node->index) {
        return strp;
    }

    do {
        node = node->next;
    }
    while (index < node->index);

    list_remove(active);
    list_insert(active, node);

    return strp;
}

char *pool::sprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *strp = vsprintf(fmt, ap);
    va_end(ap);
    return strp;
}

struct pool_module {
    int module_init() {
        pool::_global = pool::create(page_allocator::global());
        return 0;
    }
};

ll_module(pool_module);
};
