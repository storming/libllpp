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
    typename free_checker<typename member_of<_T>::class_type>::template has_signature<void(void*, size_t)>> : std::true_type {};

MEMBER_CHECKER_DECL(new_checker, _new);
template <typename _T>
using has_new = typename new_checker<_T>::has_function;

MEMBER_CHECKER_DECL(delete_checker, _delete);
template <typename _T>
using check_delete = std::integral_constant<bool, delete_checker<_T>::has_function::value>;

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
template <typename _Allocator, bool = true>
struct _new_helper {
    template <typename _T, typename ..._Args>
    static _T *_new(_Allocator &a, _Args&&...args) {
        return a._new<_T>(std::forward<_Args>(args)...);
    }

    template <typename _T, typename ..._Args>
    static _T *_new(const _Allocator &a, _Args&&...args) {
        return a._new<_T>(std::forward<_Args>(args)...);
    }
};

template <typename _Allocator>
struct _new_helper<_Allocator, !has_new<_Allocator>::value> {
    template <typename _T, typename ..._Args>
    static _T *_new(_Allocator &a, _Args&&...args) {
        return ::new(a.alloc(sizeof(_T))) _T(std::forward<_Args>(args)...);
    }

    template <typename _T, typename ..._Args>
    static _T *_new(const _Allocator &a, _Args&&...args) {
        return ::new(a.alloc(sizeof(_T))) _T(std::forward<_Args>(args)...);
    }
};

template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<std::is_class<_Allocator>::value, _T>::type*
_new(_Allocator *a, _Args&&...args) {
    return _new_helper<_Allocator>::template _new<_T>(*a, std::forward<_Args>(args)...);
}

template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<std::is_class<_Allocator>::value, _T>::type*
_new(_Allocator &a, _Args&&...args) {
    return _new_helper<_Allocator>::template _new<_T>(a, std::forward<_Args>(args)...);
}

template <typename _T, typename _Allocator, typename ..._Args>
inline typename std::enable_if<std::is_class<_Allocator>::value, _T>::type*
_new(const _Allocator &a, _Args&&...args) {
    return _new_helper<_Allocator>::template _new<_T>(a, std::forward<_Args>(args)...);
}

/* delete */
template <typename _Allocator, bool = true>
struct _delete_helper {
    template <typename _T>
    static void _delete(_Allocator &a, void *p) {
        a._delete<_T>(p);
    }
    template <typename _T>
    static void _delete(const _Allocator &a, void *p) {
        a._delete<_T>(p);
    }
};

template <typename _Allocator>
struct _delete_helper<_Allocator, !check_delete<_Allocator>::value> {
    template <typename _T>
    static void _delete(_Allocator &a, void *p) {
        if (p) {
            destroy<_T>(p);
            _free(std::forward<_Allocator>(a), p, sizeof(_T));
        }
    }
    template <typename _T>
    static void _delete(const _Allocator &a, void *p) {
        if (p) {
            destroy<_T>(p);
            _free(a, p, sizeof(_T));
        }
    }
};

template <typename _T, typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type
_delete(_Allocator *a, void *p) {
    _delete_helper<_Allocator>::template _delete<_T>(*a, p);
}

template <typename _T, typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type
_delete(_Allocator &a, _T *p) {
    _delete_helper<_Allocator>::template _delete<_T>(a, p);
}

template <typename _T, typename _Allocator> 
inline typename std::enable_if<std::is_class<_Allocator>::value, void>::type
_delete(const _Allocator &a, _T *p) {
    _delete_helper<_Allocator>::template _delete<_T>(a, p);
}

}

#endif

