#ifndef __LIBLLPP_MALLOCATOR_H__
#define __LIBLLPP_MALLOCATOR_H__

#include "cache.h"
#include <cstdlib>

namespace ll {
class mallocator {
    static mallocator _instance;
public:
    static constexpr bool has_free = true;
    mallocator() noexcept {}
    mallocator(const mallocator&) noexcept {}

    void *alloc(size_t size) noexcept {
        return caches::get(size)->alloc();
    }

    void *alloc(size_t size) const noexcept {
        return caches::get(size)->alloc();
    }

    void free(void *p, size_t size) noexcept {
        caches::get(size)->free(p);
    }

    void free(void *p, size_t size) const noexcept {
        caches::get(size)->free(p);
    }

    static mallocator &instance() {
        return _instance;
    }
};

}

#endif
