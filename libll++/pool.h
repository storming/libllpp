#ifndef __LIBLLPP_POOL_H__
#define __LIBLLPP_POOL_H__

#include <cstring>
#include <cstdarg>

#include "list.h"
#include "friend.h"
#include "page.h"

namespace ll {

struct pool_entry {
    list_entry _entry;
};

class pool : public pool_entry {
    friend class pool_module;
    friend class pool_vbuff;
private:
    typedef ll_list(pool_entry, _entry) pool_list_t;
    pool *_parent;
    page_allocator *_allocator;
    pool_list_t _children;
    page *_self;
    page *_active;
    char *_firstp;
    static pool *_global;
    void *alloc(page *active, size_t size);
public:
    static pool *create(pool *parent, page_allocator *allocator = nullptr);
    static pool *create(page_allocator *allocator);
    static void destroy(pool *p);
    void clear();
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

    page_allocator *allocator() {
        return _allocator;
    }

    static pool *global() {
        return _global;
    }

    static pool *instance() {
        return _global;
    }

    char *strdup(const char *str, size_t n);
    char *strdup(const char *str);
    char *vsprintf(const char *fmt, va_list ap);
    char *sprintf(const char *fmt, ...);
};

};

#endif

