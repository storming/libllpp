#ifndef __LIBLLPP_MEMORY_H__
#define __LIBLLPP_MEMORY_H__

#include <utility>
#include <cassert>
#include <new>

#include "pool.h"
#include "obstack.h"
#include "cache.h"
#include "member.h"

namespace ll {

MEMBER_CHECKER_DECL(alloc_checker, alloc);
MEMBER_CHECKER_DECL(free_checker, free);

template <typename _T>
using check_alloc = std::integral_constant<bool, alloc_checker<_T>::template has_signature<void*(size_t)>::value>;

template <typename _T>
using check_free = std::integral_constant<bool, free_checker<_T>::template has_signature<void(void*, size_t)>::value>;

template <typename _T, typename _TAllocator = pool>
struct factory {
    typedef _T type_t;
    typedef _TAllocator allocator_t;

    template <typename ..._Args>
    static type_t *create(allocator_t *a, _Args &&... args) {
        void *p = a->alloc(sizeof(type_t));
        return new(p) type_t(std::forward<_Args>(args)...);
    }

    static void destroy(type_t *p, allocator_t *a) {
        if (p) {
            ((type_t*)p)->~type_t();
        }
    }
};

template <typename _T>
struct factory<_T, cache> {
    typedef _T type_t;
    typedef cache allocator_t;

    template <typename ..._Args>
    static type_t *create(allocator_t *a, _Args &&... args) {
        assert(a->matching<_T>());
        void *p = a->alloc();
        return new(p) type_t(std::forward<_Args>(args)...);
    }

    static void destroy(type_t *p, allocator_t *a) {
        if (p) {
            ((type_t*)p)->~type_t();

            a->free(p);
        }
    }
};

template <typename _T>
struct factory<_T, caches> {
    typedef _T type_t;
    typedef caches allocator_t;

    template <typename ..._Args>
    static type_t *create(allocator_t *a, _Args &&... args) {
        void *p = a->alloc(sizeof(type_t));
        return new(p) type_t(std::forward<_Args>(args)...);
    }

    static void destroy(type_t *p, allocator_t *a) {
        if (p) {
            ((type_t*)p)->~type_t();
            a->free(p, sizeof(type_t));
        }
    }
};

class malloctor {
public:
    static void *alloc(size_t size);
    static void free();
};

template <typename _T>
struct factory<_T, void> {
public:
    typedef _T type_t;
    typedef caches allocator_t;
private:
    struct alloc_type {
        size_t _size;
        type_t _obj;
    };
public:
    template <typename ..._Args>
    static type_t *create(allocator_t *a, _Args &&... args) {
        alloc_type *p = a->alloc(sizeof(alloc_type));
        p->_size = sizeof(alloc_type);

        return new(&p->_obj) type_t(std::forward<_Args>(args)...);
    }

    static void destroy(type_t *p, allocator_t *a) {
        if (p) {
            p->~type_t();
            alloc_type *at = containerof_member(p, &alloc_type::_obj);
            a->free(at, at->size);
        }
    }

    template <typename ..._Args>
    static type_t *create(_Args &&... args) {
        alloc_type *p = caches::global()->alloc(sizeof(alloc_type));
        p->_size = sizeof(alloc_type);

        return new(&p->_obj) type_t(std::forward<_Args>(args)...);
    }

    static void destroy(type_t *p) {
        if (p) {
            p->~type_t();
            alloc_type *at = containerof_member(p, &alloc_type::_obj);
            caches::global()->free(at, at->size);
        }
    }
};

template <typename _T>
struct factory_of {
    template<typename A, typename = void>
    struct get_type : std::false_type {};

    template<typename A>
    struct get_type<A> : std::true_type {
        typedef A type;
    };

    template<typename A, typename = std::true_type>
    struct get_factory {
        typedef factory<_T, pool> type;
    };

    template<typename A>
    struct get_factory<A, std::integral_constant<bool, get_type<typename A::factory_t>::value>> {
        typedef typename A::factory_t type;
    };

    typedef typename get_factory<_T>::type type;
};


template <typename _T, typename _Factory = typename factory_of<_T>::type>
struct factory_bind final : public _T {
    typedef _T type_t;
    typedef _Factory factory_t;
};

template <typename _T, typename _Factory = typename factory_of<_T>::type, typename _Allocator = typename _Factory::allocator_t, typename ..._Args>
inline _T *create(_Allocator *allocator, _Args &&... args) {
    return static_cast<_T*>(_Factory::create(allocator, std::forward<_Args>(args)...));
}

template <typename _T, typename _Factory = typename factory_of<_T>::type, typename _Allocator = typename _Factory::allocator_t>
inline void destroy(_T *p, _Allocator *allocator) {
    _Factory::destroy(p, allocator);
}

template <typename _T, typename ..._Args>
inline _T *create(_Args &&... args) {
    return static_cast<_T*>(factory<_T, void>::create(std::forward<_Args>(args)...));
}

template <typename _T>
inline void destroy(_T *p) {
    p->~_T();
}

template <typename _Allocator, typename _T, typename ..._Args>
inline _T *_new(_Allocator *allocator, _Args&&...args) {
    return allocator->_new<_T>(std::forward<_Args>(args)...);
}

}

#endif

