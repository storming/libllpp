#ifndef __LIBLLPP_MEMORY_H__
#define __LIBLLPP_MEMORY_H__

#include <cstddef>

namespace ll {
inline void *mem_alloc(std::nullptr_t, size_t size) noexcept;
inline void mem_free(std::nullptr_t, void *p, size_t size) noexcept;
}

#include "construct.h"
#include "malloc_allocator.h"
#include "new_allocator.h"
#include "mallocator.h"
#include "allocator.h"
#include "page.h"
#include "cache.h"
#include "pool.h"
#include "obstack.h"

namespace ll {

inline void *mem_alloc(size_t size) noexcept
{
    return caches::get(size)->alloc();
}

template <typename _T>
inline void *mem_alloc()
{
    return caches::get(sizeof(_T))->alloc();
}

inline void mem_free(void *p, size_t size) noexcept 
{
    caches::get(size)->free(p);
}

template <typename _T>
inline void mem_free(_T *p) 
{
    caches::get(sizeof(_T))->free(p);
}

inline void *mem_alloc(std::nullptr_t, size_t size) noexcept {
    return mem_alloc(size);
};

inline void mem_free(std::nullptr_t, void *p, size_t size) noexcept {
    mem_free(p, size);
};


}
#endif

