#ifndef __LIBLLPP_MALLOC_ALLOCATOR_H__
#define __LIBLLPP_MALLOC_ALLOCATOR_H__

#include <cstdlib>

namespace ll {

class malloc_allocator {
public:
    static constexpr bool has_free = true;

    malloc_allocator() noexcept {}
    malloc_allocator(const malloc_allocator&) noexcept {}

    void *alloc(size_t size) noexcept {
        return std::malloc(size);
    }

    void *alloc(size_t size) const noexcept {
        return std::malloc(size);
    }

    void free(void *p, size_t) noexcept {
        std::free(p);
    }

    void free(void *p, size_t) const noexcept {
        std::free(p);
    }
};

}


#endif
