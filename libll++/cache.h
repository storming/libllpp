#ifndef __LIBLLPP_CACHE_H__
#define __LIBLLPP_CACHE_H__

#include "list.h"
#include "pool.h"
#include "etc.h"
#include "friend.h"

namespace ll {

class cache {
private:
    struct node {
        slist_entry _entry;
    };
    unsigned _size;
    pool *_pool;
    ll_list(node, _entry) _freelist;
public:
    cache(unsigned size, pool *p) : _size(ll_align_default(size)), _pool(p), _freelist() {
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

    void free(void *p) {
        _freelist.push_front((node*)p);
    }
};

template <unsigned __size> 
class sized_cache : public cache {
public:
    sized_cache(pool *p) : cache(__size, p) {}
};

template <typename _T>
using typed_cache = sized_cache<sizeof(_T)>;

class caches {
    friend class cache_module;
public:
    static constexpr unsigned max_index = 32;
private:
    cache *_caches;
    static caches *_global;
public:
    caches(pool *p);

    cache *get(unsigned size) {
        assert(size);
        unsigned index = (ll_align_default(size) >> ll_align_order) - 1;
        assert(index < max_index);
        return _caches + index;
    }

    void *alloc(unsigned size) {
        return get(size)->alloc();
    }

    void free(void *p, unsigned size) {
        return get(size)->free(p);
    }

    static caches *global() {
        return _global;
    }
};

}


#endif

