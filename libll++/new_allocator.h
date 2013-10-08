#ifndef __LIBLLPP_NEW_ALLOCATOR_H__
#define __LIBLLPP_NEW_ALLOCATOR_H__

#include <cstdlib>
#include "cache.h"

namespace ll {

class new_allocator {
private:
    struct alloc_type {
        size_t _size;
        char _obj;
    };

public:
    static constexpr bool has_free = true;

    new_allocator() noexcept {}
    new_allocator(const new_allocator&) noexcept {}

    void *alloc(size_t size) noexcept {
        size += (sizeof(alloc_type) - 1);
        alloc_type *p = (alloc_type*)caches::get(size)->alloc();
        p->_size = size;
        return &p->_obj;
    }

    void free(void *p, size_t) noexcept {
        alloc_type *at = containerof_member((char*)p, &alloc_type::_obj);
        caches::get(at->_size)->free(at);
    }

    void *alloc(size_t size) const noexcept {
        size += (sizeof(alloc_type) - 1);
        alloc_type *p = (alloc_type*)caches::get(size)->alloc();
        p->_size = size;
        return &p->_obj;
    }

    void free(void *p, size_t) const noexcept {
        alloc_type *at = containerof_member((char*)p, &alloc_type::_obj);
        caches::get(at->_size)->free(at);
    }

};

}

#endif

