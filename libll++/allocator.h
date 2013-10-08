#ifndef __LIBLLPP_ALLOCATOR_H__
#define __LIBLLPP_ALLOCATOR_H__

#include "alloc_checker.h"
#include "construct.h"

namespace ll {

template <typename _Allocator>
class allocator {
public:
    typedef _Allocator allocator_t;

private:
    allocator_t *_a;

public:
    explicit allocator() : _a() {}
    explicit allocator(allocator_t *ma) : _a(ma) {}
    explicit allocator(const allocator &o) : _a(o._a) {}

    void *alloc(size_t size) noexcept {
        return _a->alloc(size);
    }

    void *alloc(size_t size) const noexcept {
        return _a->alloc(size);
    }

    void free(void *p, size_t size) noexcept {
        _a->free(p, size);
    }

    void free(void *p, size_t size) const noexcept {
        _a->free(p, size);
    }

};

}

#endif

