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
inline _T *construct(void *p, _Args&&...args) noexcept {
    return ::new(p) _T(std::forward<_Args>(args)...);
}

template <typename _T, typename _U>
inline void destroy(_U *p) noexcept {
    static_cast<_T*>(p)->~_T();
}

/* alloc */
template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void*>::type
mem_alloc(_Allocator *a, size_t size) noexcept {
    return a->alloc(size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void*>::type
mem_alloc(_Allocator &a, size_t size) noexcept {
    return a.alloc(size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void*>::type 
mem_alloc(const _Allocator &a, size_t size) noexcept {
    return a.alloc(size);
};

/* free */
template <typename _Allocator, typename = std::true_type>
struct _free_helper {
    static void free(_Allocator &a, void *p, size_t size) noexcept {
        return a.free(p, size);
    }
    static void free(const _Allocator &a, void *p, size_t size) noexcept {
        return a.free(p, size);
    }
};

template <typename _Allocator>
struct _free_helper<_Allocator, std::integral_constant<bool, !has_free<_Allocator>::value>> {
    static void free(_Allocator &a, void *p, size_t size) noexcept {
    }
    static void free(const _Allocator &a, void *p, size_t size) noexcept {
    }
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type 
mem_free(_Allocator &a, void *p, size_t size) noexcept {
    return _free_helper<_Allocator>::free(a, p, size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type 
mem_free(const _Allocator &a, void *p, size_t size) noexcept {
    return _free_helper<_Allocator>::free(a, p, size);
};

template <typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type 
mem_free(_Allocator *a, void *p, size_t size) noexcept {
    return _free_helper<_Allocator>::free(*a, p, size);
};

/* new */
MEMBER_CHECKER_DECL(new_checker, _new);
MEMBER_CHECKER_DECL(new_object_checker, new_object);
template <typename _T>
struct has_member_new {
    typedef typename std::remove_pointer<typename std::decay<_T>::type>::type type;
    
    template <typename _A, typename = std::false_type>
    struct has : new_object_checker<_A>::has_function {};

    template <typename _A>
    struct has<_A, std::integral_constant<bool, std::is_class<_A>::value>> : std::false_type {};

    static constexpr bool value = has<type>::value;
};

template <typename _T>
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

template <typename _T, typename ..._Args>
struct is_static_template_new {
    template <typename _A, typename = std::true_type>
    struct check : std::false_type {};

    template <typename _A>
    struct check<_A, std::integral_constant<bool, 
        got_type<decltype(_A::template _new<_A>(std::declval<_Args>()...))>::value>> : std::true_type {};

    static constexpr bool value = check<_T>::value;
};


/* _T static _new */
template <typename _T, typename ..._Args>
inline typename std::enable_if<
        (has_static_new<_T>::value && 
         is_static_template_new<_T, _Args...>::value),
        _T*
    >::type
_new(_Args&&...args) noexcept {
    return _T::template _new<_T>(std::forward<_Args>(args)...);
}

template <typename _T, typename ..._Args>
inline typename std::enable_if<
        (has_static_new<_T>::value && 
         !is_static_template_new<_T, _Args...>::value),
        _T*
    >::type
_new(_Args&&...args) noexcept {
    return _T::_new(std::forward<_Args>(args)...);
}

/* allocator _new */
template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_new<_T>::value &&
         has_member_new<_Allocator>::value), 
        _T*
    >::type
_new(_Allocator &&a, _Args&&...args) noexcept {
    return a._new<_T>(std::forward<_Args>(args)...);
}

template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_new<_T>::value &&
         has_member_new<_Allocator>::value), 
        _T*
    >::type
_new(_Allocator *a, _Args&&...args) noexcept {
    return a->_new<_T>(std::forward<_Args>(args)...);
}

/* default _new */
template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<
        (!has_static_new<_T>::value &&
         !has_member_new<_Allocator>::value), 
        _T*
    >::type
_new(_Allocator &&a, _Args&&...args) noexcept {
    return ::new(mem_alloc(std::forward<_Allocator>(a), sizeof(_T))) _T(std::forward<_Args>(args)...);
}

/* delete */
MEMBER_CHECKER_DECL(delete_checker, _delete);
MEMBER_CHECKER_DECL(delete_object_checker, delete_object);
template <typename _T>
struct has_member_delete {
    typedef typename std::remove_pointer<typename std::decay<_T>::type>::type type;

    template <typename _A, typename = std::false_type>
    struct has : delete_object_checker<type>::has_function {};

    template <typename _A>
    struct has<_A, std::integral_constant<bool, std::is_class<_A>::value>> : std::false_type {};

    static constexpr bool value = has<type>::value;

    //typedef typename std::remove_pointer<typename std::decay<_T>::type>::type type;
    //static constexpr bool value = delete_object_checker<type>::has_function::value;
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
template <typename _T, typename _U>
inline typename std::enable_if<has_static_delete<_T>::value, void>::type
_delete(_U *p) noexcept {
    if (p) {
        _T::_delete(static_cast<_T*>(p));
    }
}

template <typename _T, typename _Allocator, typename _U>
inline typename std::enable_if<has_static_delete<_T>::value, void>::type
_delete(_Allocator&& a, _U *p) noexcept {
    if (p) {
        _T::_delete(std::forward<_Allocator>(a), static_cast<_T*>(p));
    }
}

/* allocator _delete */
template <typename _T, typename _Allocator, typename _U>
inline typename std::enable_if<
        (!has_static_delete<_T>::value && 
         has_member_delete<_Allocator>::value), 
        void
    >::type
_delete(_Allocator &&a, _U *p) noexcept {
    if (p) {
        a._delete<_T>(static_cast<_T*>(p));
    }
}

template <typename _T, typename _Allocator, typename _U>
inline typename std::enable_if<
        (!has_static_delete<_T>::value && 
         has_member_delete<_Allocator>::value), 
        void
    >::type
_delete(_Allocator *a, _U *p) noexcept {
    if (p) {
        a->_delete<_T>(static_cast<_T*>(p));
    }
}

/* default _delete */
template <typename _T, typename _Allocator, typename _U>
inline typename std::enable_if<
        (!has_static_delete<_T>::value && 
         !has_member_delete<_Allocator>::value), 
        void
    >::type
_delete(_Allocator &&a, _U *p) noexcept {
    if (p) {
        destroy<_T>(p);
        mem_free(std::forward<_Allocator>(a), p, sizeof(_T));
    }
}

}

#endif

