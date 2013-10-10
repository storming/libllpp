#ifndef __LIBLLPP_CONSTRUCT_H__
#define __LIBLLPP_CONSTRUCT_H__

#include <new>
#include <utility>
#include <cstdlib>
#include "member.h"

namespace ll {

MEMBER_CHECKER_DECL(free_checker, free);
template <typename _T, typename = std::false_type, typename = std::false_type>
struct has_free : std::false_type {};

template <typename _T>
struct has_free<
    _T, 
    typename free_checker<_T>::has_function,
    typename free_checker<typename member_of<decltype(&_T::free)>::class_type>::
        template has_signature<void(void*, size_t)>> : std::true_type {};

template <typename _T, typename ..._Args>
inline _T *construct(void *p, _Args&&...args) {
    return ::new(p) _T(std::forward<_Args>(args)...);
}

template <typename _T>
inline void destroy(void *p) {
    static_cast<_T*>(p)->~_T();
}

/* alloc */
template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void*>::type
_alloc(_Allocator *a, size_t size) {
    return a->alloc(size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void*>::type
_alloc(_Allocator &a, size_t size) {
    return a.alloc(size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void*>::type 
_alloc(const _Allocator &a, size_t size) {
    return a.alloc(size);
};

/* free */
template <typename _Allocator, typename = std::true_type>
struct _free_helper {
    static void free(_Allocator &a, void *p, size_t size) {
        return a.free(p, size);
    }
    static void free(const _Allocator &a, void *p, size_t size) {
        return a.free(p, size);
    }
};

template <typename _Allocator>
struct _free_helper<_Allocator, std::integral_constant<bool, !has_free<_Allocator>::value>> {
    static void free(_Allocator &a, void *p, size_t size) {
    }
    static void free(const _Allocator &a, void *p, size_t size) {
    }
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type 
_free(_Allocator &a, void *p, size_t size) {
    return _free_helper<_Allocator>::free(a, p, size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type 
_free(const _Allocator &a, void *p, size_t size) {
    return _free_helper<_Allocator>::free(a, p, size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type 
_free(_Allocator *a, void *p, size_t size) {
    return _free_helper<_Allocator>::free(*a, p, size);
};

/* new */
MEMBER_CHECKER_DECL(new_checker, _new);
template <typename _T, typename ..._Args>
struct has_member_new {
    typedef typename std::remove_pointer<typename std::decay<_T>::type>::type type;
    static constexpr bool value = new_checker<type>::has_function::value;
};

template <typename _T, typename ..._Args>
struct has_static_new {
    template <typename _A, typename = std::true_type>
    struct check : std::true_type {};

    template <typename _A>
    struct check<_A, std::integral_constant<bool, std::is_member_pointer<decltype(&_A::_new)>::value>> : std::false_type {};

    template <typename _A, typename = std::false_type>
    struct has : new_checker<_A>::has {};
    
    template <typename _A>
    struct has<_A, std::integral_constant<bool, std::is_class<_A>::value>> : std::false_type {};

    static constexpr bool value = has<_T>::value && check<_T>::value;
};

/* _T static _new */
template <typename _T, typename ..._Args>
inline typename std::enable_if<
        has_static_new<_T, _Args&&...>::value,
        _T*
    >::type
_new(_Args&&...args) {
    return _T::_new(std::forward<_Args>(args)...);
}

/* allocator _new */
template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_new<_T, _Allocator&&, _Args&&...>::value &&
         has_member_new<_Allocator, _Args&&...>::value), 
        _T*
    >::type
_new(_Allocator &&a, _Args&&...args) {
    return a._new<_T>(std::forward<_Args>(args)...);
}

template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_new<_T, _Allocator&&, _Args&&...>::value &&
         has_member_new<_Allocator, _Args&&...>::value), 
        _T*
    >::type
_new(_Allocator *a, _Args&&...args) {
    return a->_new<_T>(std::forward<_Args>(args)...);
}

/* default _new */
template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_new<_T, _Allocator&&, _Args&&...>::value &&
         !has_member_new<_Allocator, _Args&&...>::value), 
        _T*
    >::type
_new(_Allocator &&a, _Args&&...args) {
    return ::new(_alloc(std::forward<_Allocator>(a), sizeof(_T))) _T(std::forward<_Args>(args)...);
}


/* delete */
MEMBER_CHECKER_DECL(delete_checker, _delete);
template <typename _T>
struct has_member_delete {
    typedef typename std::remove_pointer<typename std::decay<_T>::type>::type type;
    static constexpr bool value = delete_checker<type>::has_function::value;
};

template <typename _T>
struct has_static_delete {
    template <typename _A, typename = std::true_type>
    struct check : std::true_type {};

    template <typename _A>
    struct check<_A, std::integral_constant<bool, std::is_member_pointer<decltype(&_A::_delete)>::value>> : std::false_type {};

    template <typename _A, typename = std::false_type>
    struct has : delete_checker<_A>::has {};

    template <typename _A>
    struct has<_A, std::integral_constant<bool, std::is_class<_A>::value>> : std::false_type {};

    static constexpr bool value = has<_T>::value && check<_T>::value;
};

/* _T static _delete */
template <typename _T>
inline typename std::enable_if<has_static_delete<_T>::value, void>::type
_delete(void *p) {
    if (p) {
        _T::template _delete(static_cast<_T*>(p));
    }
}

template <typename _T, typename _Allocator>
inline typename std::enable_if<has_static_delete<_T>::value, void>::type
_delete(_Allocator&& a, void *p) {
    if (p) {
        _T::template _delete(std::forward<_Allocator>(a), static_cast<_T*>(p));
    }
}

/* allocator _delete */
template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_delete<_T>::value && 
         has_member_delete<_Allocator>::value), 
        void
    >::type
_delete(_Allocator &&a, void *p) {
    if (p) {
        a._delete<_T>(static_cast<_T*>(p));
    }
}

template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_delete<_T>::value && 
         has_member_delete<_Allocator>::value), 
        void
    >::type
_delete(_Allocator *a, void *p) {
    if (p) {
        a->_delete<_T>(static_cast<_T*>(p));
    }
}

/* default _delete */
template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_delete<_T>::value && 
         !has_member_delete<_Allocator>::value), 
        void
    >::type
_delete(_Allocator &&a, void *p) {
    if (p) {
        destroy<_T>(p);
        _free(std::forward<_Allocator>(a), p, sizeof(_T));
    }
}

}

#endif

