#ifndef __LIBLLPP_POOL_H__
#define __LIBLLPP_POOL_H__

#include <cstdarg>
#include <utility>

#include "list.h"
#include "page.h"
#include "closure.h"
#include "construct.h"

namespace ll {

struct pool_entry {
    list_entry _entry;
};

class pool : protected pool_entry {
    friend class pool_module;
    friend class pool_vbuff;
private:
    typedef closure<void(pool&)> closure_t;
    typedef ll_list(pool_entry, _entry) pool_list_t;

public:
    class stub {
        friend class pool;
        clist_entry _entry;
        closure_t *_closure;
    public:
        stub() : _entry(nullptr) {}
    };

private:
    pool *_parent;
    page_allocator *_allocator;
    pool_list_t _children;
    page *_self;
    page *_active;
    char *_firstp;
    ll_list(stub, _entry) _destroy_list;

    static pool *_global;
    void *alloc(page *active, size_t size);
    void emit_destroy_callback();
    pool() {}
    ~pool() {}

public:
    void *alloc(size_t size) {
        size = ll_align_default(size);
        page *pg = _active;
        if (ll_likely(size <= pg->space())) {
            void *p = pg->firstp;
            pg->firstp += size;
            return p;
        }
        return alloc(pg, size);
    }

    void *calloc(size_t size) {
        void *p = alloc(size);
        memset(p, 0, size);
        return p;
    }

    template <typename _T, typename ..._Params>
    stub *connect(_T &&obj, _Params&&...args) noexcept {
        stub *s = _new<stub>(this);
        s->_closure = closure_t::_new(this, std::forward<_T>(obj), std::forward<_Params>(args)...);
        _destroy_list.push_back(s);
        return nullptr;
    };

    void disconnect(stub *s) {
        _destroy_list.remove(s);
    }

    page_allocator *allocator() {
        return _allocator;
    }

    static pool *global() {
        return _global;
    }

    static pool *create(pool *parent, page_allocator *allocator = nullptr);
    static pool *create(page_allocator *allocator);
    static void destroy(pool *p);
    void clear();

    char *strdup(const char *str, size_t n);
    char *strdup(const char *str);
    char *vsprintf(const char *fmt, va_list ap);
    char *sprintf(const char *fmt, ...);
};

}

#endif

