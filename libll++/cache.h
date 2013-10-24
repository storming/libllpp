#ifndef __LIBLLPP_CACHE_H__
#define __LIBLLPP_CACHE_H__

#include "list.h"
#include "pool.h"
#include "etc.h"

namespace ll {

template <typename _Allocator>
class memory_cache {
private:
    typedef _Allocator allocator_type;
    struct node {
        slist_entry _entry;
    };
    unsigned _size;
    allocator_type _a;
    ll_list(node, _entry) _freelist;
public:
    memory_cache(unsigned size, const allocator_type &a) noexcept : 
        _size(ll_align_default(size)), 
        _a(a), 
        _freelist() {
        assert(size);
    }

    memory_cache(const memory_cache &x) noexcept :
        _size(x._size),
        _a(x._a),
        _freelist(x._freelist) {
    }

    memory_cache(memory_cache &&x) noexcept :
        _a(std::move(x._a)),
        _freelist(std::move(x._freelist)) {
        std::swap(_size, x._size);
    }

    memory_cache &operator=(const memory_cache &x) noexcept {
        memory_cache(x).swap(*this);
        return *this;
    }

    memory_cache &operator=(memory_cache &&x) noexcept {
        swap(x);
        return *this;
    }

    void swap(memory_cache &x) noexcept {
        std::swap(_size, x._size);
        std::swap(_a, x._a);
        std::swap(_freelist, x._freelist);
    }

    void clear() noexcept {
        node *p;
        while ((p = _freelist.pop_front())) {
            _free(_a, p, _size);
        }
    }

    unsigned size() noexcept {
        return _size;
    }

    bool operator==(unsigned size) noexcept {
        return _size == ll_align_default(size);
    }

    bool operator==(memory_cache &x) noexcept {
        return _size == x._size;
    }

    void *alloc() noexcept {
        void *p = (void*)_freelist.pop_front();
        if (!p) {
            p = _a.alloc(_size);
        }
        return p;
    }

    void *alloc(size_t size) noexcept {
        assert(*this == size);
        return alloc();
    }

    void free(void *p) noexcept {
        _freelist.push_front((node*)p);
    }

    void free(void *p, size_t size) noexcept {
        assert(*this == size);
        free(p);
    }
};

typedef memory_cache<pool_allocator> cache;

class caches {
    friend class caches_module;
private:
    static caches *_instance;
    cache *_caches;
    caches() noexcept;
public:
    static constexpr unsigned max_index = 128;

    static cache *get(unsigned size) noexcept {
        assert(size);
        unsigned index = (ll_align_default(size) >> ll_align_order) - 1;
        assert(index < max_index);
        return _instance->_caches + index;
    }
};

}

#endif

