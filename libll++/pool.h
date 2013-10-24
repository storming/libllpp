#ifndef __LIBLLPP_POOL_H__
#define __LIBLLPP_POOL_H__

#include <cstdarg>
#include <utility>

#include "list.h"
#include "closure.h"
#include "construct.h"
#include "page.h"
#include "allocator_bind.h"

namespace ll {

struct pool_impl_base {
    list_entry _entry;
};

class pool_impl : pool_impl_base {
    friend class pool_module;
public:
    typedef closure<void()> closure_type;
private:
    struct stub {
        clist_entry _entry;
        closure_type *_closure;
        stub() : _entry(nullptr) {}
    };

    pool_impl *_parent;
    page_allocator *_pa;
    page *_self;
    page *_active;
    char *_firstp;
    ll_list(pool_impl_base, _entry) _children;
    ll_list(stub, _entry) _destroylist;
    long _use_count;
    static pool_impl *_global;

    void init(page *pg, pool_impl *parent, page_allocator *pa) noexcept;
    void emit_destroy() noexcept;
    void *alloc(page *active, size_t size) noexcept;

    pool_impl() {}
    ~pool_impl() {}
public:
    void clear() noexcept;
    void *alloc(size_t size) noexcept {
        size = ll_align_default(size);
        page *pg = _active;
        if (ll_likely(size <= pg->space())) {
            void *p = pg->firstp;
            pg->firstp += size;
            return p;
        }
        return alloc(pg, size);
    }

    void *calloc(size_t size) noexcept {
        void *p = alloc(size);
        memset(p, 0, size);
        return p;
    }

    void attach() noexcept {
        _use_count++;
    }

    void deattach() noexcept {
        _use_count--;
        if (!_use_count) {
            ll::_delete<pool_impl>(this);
        }
    }

    pool_impl *get_parent() noexcept {
        return _parent;
    }

    page_allocator *get_page_allocator() noexcept {
        return _pa;
    }

    template <typename _T, typename ..._Params>
    void *connect(_T &&obj, _Params&&...args) noexcept {
        stub *s = ll::_new<stub>(this);
        s->_closure = ll::_new<closure_type>(this, std::forward<_T>(obj), std::forward<_Params>(args)...);
        _destroylist.push_back(s);
        return s;
    };

    void disconnect(void *s) noexcept {
        _destroylist.remove(static_cast<stub*>(s));
    }

    char *strdup(const char *str, size_t n) noexcept;
    char *strdup(const char *str) noexcept;
    char *vsprintf(const char *fmt, va_list ap) noexcept;
    char *sprintf(const char *fmt, ...) noexcept {
        va_list ap;
        va_start(ap, fmt);
        char *strp = vsprintf(fmt, ap);
        va_end(ap);
        return strp;
    }

    static pool_impl *_new(pool_impl*, page_allocator* = nullptr) noexcept;
    static pool_impl *_new(page_allocator*) noexcept;
    static void _delete(pool_impl*) noexcept;

    static pool_impl *global() noexcept {
        return _global;
    }
};

typedef pool_impl pool;
typedef allocator_bind<pool> pool_allocator;

}
#endif

