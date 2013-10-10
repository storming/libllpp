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

class pool;
template <typename _T, typename _Default = allocator<pool>>
struct allocator_of {
    template<typename A, typename = void>
    struct get_type : std::false_type {};

    template<typename A>
    struct get_type<A> : std::true_type {
        typedef A type;
    };

    template<typename A, typename = std::true_type>
    struct get_allocator {
        typedef allocator<_Default> type;
    };

    template<typename A>
    struct get_allocator<A, std::integral_constant<bool, get_type<typename A::allocator_type>::value>> {
        typedef typename A::allocator_type type;
    };

    typedef typename get_allocator<_T>::type type;
};

}

#endif

