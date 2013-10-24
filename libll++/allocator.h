#ifndef __LIBLLPP_ALLOCATOR_H__
#define __LIBLLPP_ALLOCATOR_H__

#include "allocator_bind.h"
#include "mallocator.h"

namespace ll {

typedef mallocator allocator;

struct dumb_allocator {};


/*
template <typename _T, typename _Default = allocator_bind<pool>>
struct allocator_of {
    template<typename A, typename = std::true_type>
    struct get_allocator {
        typedef _Default type;
    };

    template<typename A>
    struct get_allocator<A, std::integral_constant<bool, got_type<typename A::allocator_type>::value>> {
        typedef typename A::allocator_type type;
    };

    typedef typename get_allocator<_T>::type type;
};
*/

}

#endif

