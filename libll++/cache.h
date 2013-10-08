#ifndef __LIBLLPP_CACHE_H__
#define __LIBLLPP_CACHE_H__

#include <new>
#include "list.h"
#include "pool.h"
#include "etc.h"

namespace ll {

template <typename _Allocator = pool>
class pool_cache {
public:
    typedef _Allocator allocator_t;
private:
    struct node {
        slist_entry _entry;
    };
    unsigned _size;
    allocator_t *_pool;
    ll_list(node, _entry) _freelist;
public:
    pool_cache(unsigned size, allocator_t *p) : _size(ll_align_default(size)), _pool(p), _freelist() {
        assert(size);
    }

    unsigned size() {
        return _size;
    }

    bool matching(unsigned size) {
        return _size == ll_align_default(size);
    }

    template <typename _T>
    bool matching() {
        return _size == ll_align_default(sizeof(_T));
    }

    void *alloc() {
        void *p = (void*)_freelist.pop_front();
        if (!p) {
            p = _pool->alloc(_size);
        }
        return p;
    }

    void *alloc(size_t size) {
        assert(matching(size));
        return alloc();
    }

    void free(void *p) {
        _freelist.push_front((node*)p);
    }

    void free(void *p, size_t size) {
        assert(matching(size));
        free(p);
    }
};

template <typename _Allocator = pool>
class pool_caches {
    friend class cache_module;
public:
    typedef _Allocator allocator_t;
    typedef pool_cache<allocator_t> cache_t;
private:
    cache_t *_caches;
    unsigned _max_index;
public:
    pool_caches(unsigned max_index, allocator_t *p) {
        assert(p);
        if (max_index < 1) {
            max_index = 1;
        }
        _max_index = max_index;

        cache_t *c;
        c = _caches = (cache_t*)p->alloc(_max_index * sizeof(cache_t));
        for (unsigned i = 0; i < _max_index; i++, c++) {
            new(c) cache_t((i + 1) << ll_align_order, p);
        }
    }

    cache_t *get(unsigned size) {
        assert(size);
        unsigned index = (ll_align_default(size) >> ll_align_order) - 1;
        assert(index < _max_index);
        return _caches + index;
    }

    template <typename _T>
    cache_t *get() {
        return get(sizeof(_T));
    }
};

class caches : protected pool_caches<pool> {
    friend class caches_module;
private:
    static caches *_instance;
    caches() : pool_caches<pool>(128, pool::global()) {}
public:
    typedef pool_caches<pool>::cache_t cache_t;
    static caches *instance() {
        return _instance;
    }

    static cache_t *get(unsigned size) {
        return static_cast<pool_caches<pool>*>(_instance)->get(size);
    }

    template <typename _T>
    static cache_t *get() {
        return static_cast<pool_caches<pool>*>(_instance)->get<_T>();
    }
};

using cache = caches::cache_t;

}

#endif

