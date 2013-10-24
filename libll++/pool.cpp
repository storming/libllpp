#include <cassert>

#include "module.h"
#include "printf.h"
#include "pool.h"

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

pool_impl *pool_impl::_global = nullptr;

/* pool_impl */
void pool_impl::init(page *pg, pool_impl *parent, page_allocator *pa) noexcept
{
    _parent = parent;
    _pa = pa;
    _self = pg;
    _active = pg;
    _children.init();
    _destroylist.init();
    _use_count = 1;

    pg->firstp = _firstp = (char *)this + ll_align_default(sizeof(pool_impl));
    if (parent) {
        parent->_children.push_front(this);
    }
}

inline void pool_impl::emit_destroy() noexcept
{
    stub *s;
    while ((s = _destroylist.pop_front())) {
        s->_closure->apply();
    }
}

void pool_impl::clear() noexcept
{
    pool_impl *child;
    while ((child = static_cast<pool_impl*>(_children.front()))) {
        ll::_delete<pool_impl>(child);
    }

    emit_destroy();

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
        _pa->free(pg);
        pg = tmp;
    }

    _self->next = _self;
    _self->ref = &_self->next;
}

void *pool_impl::alloc(page *active, size_t size) noexcept
{
    page *pg = active->next;

    if (size <= pg->space()) {
        list_remove(pg);
    }
    else {
        pg = _pa->alloc(size);
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

/* _new managed impl */
pool_impl *pool_impl::_new(pool_impl *parent, page_allocator *pa) noexcept
{
    assert(parent);

    if (!pa) {
        pa = parent->_pa;
    }

    page *pg = pa->alloc(page_allocator::page_min_size);
    pg->next = pg;
    pg->ref = &pg->next;

    pool_impl *impl = (pool_impl*)pg->firstp;
    impl->init(pg, parent, pa);
    return impl;
}

/* _new unmanaged impl */
pool_impl *pool_impl::_new(page_allocator *pa) noexcept
{
    assert(pa);

    page *pg = pa->alloc(page_allocator::page_min_size);
    pg->next = pg;
    pg->ref = &pg->next;

    pool_impl *impl = (pool_impl*)pg->firstp;
    impl->init(pg, nullptr, pa);
    return impl;
}

void pool_impl::_delete(pool_impl *impl) noexcept
{
    pool_impl *child;
    while ((child = static_cast<pool_impl*>(impl->_children.front()))) {
        ll::_delete<pool_impl>(child);
    }

    impl->emit_destroy();

    if (impl->_parent) {
        decltype(_children)::remove(impl);
    }

    page_allocator *pa = impl->_pa;
    page *tmp;
    page *pg = impl->_self;
    *pg->ref = nullptr;

    while (pg) {
        tmp = pg->next;
        pa->free(pg);
        pg = tmp;
    }
}

#define SPRINTF_MIN_STRINGSIZE 32
char *pool_impl::vsprintf(const char *fmt, va_list ap) noexcept
{
    struct vbuff : public printf_formatter::buff {
        page *_node;
        pool_impl *_owner;
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
                node = _owner->_pa->alloc(size);

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

    vbuff vb;
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
        _pa->free(node);
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

char *pool_impl::strdup(const char *str, size_t n) noexcept
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

char *pool_impl::strdup(const char *str) noexcept
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
        pool_impl::_global = _new<pool_impl>(page_allocator::global());
        return 0;
    }
};

ll_module(pool_module);
}
