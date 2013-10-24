#ifndef __LIBLLPP_FUNCTOR_H__
#define __LIBLLPP_FUNCTOR_H__

#include "member.h"
#include "tuple_apply.h"
#include "log.h"

namespace ll {

/* class_fn_impl */
template <typename _T>
class class_fn_impl {
public:
    typedef _T type;
private:
    type *_fn;

public:
    class_fn_impl(const class_fn_impl &x) noexcept 
        : _fn(x._fn) {}
    class_fn_impl(class_fn_impl &&x) noexcept 
        : _fn() {
        std::swap(_fn, x._fn);
    }
    class_fn_impl(type *fn) noexcept 
        : _fn(fn) {}
    class_fn_impl(type &fn) noexcept 
        : _fn(std::addressof(fn)) {}

    template <typename ..._Args>
    auto operator()(_Args&&...args) const noexcept -> decltype(std::declval<type>()(std::forward<_Args>(args)...)) {
        return (*_fn)(std::forward<_Args>(args)...);
    }

    template <typename ..._Args>
    auto apply(_Args&&...args) const noexcept -> decltype((*_fn)(std::forward<_Args>(args)...)) {
        return (*_fn)(std::forward<_Args>(args)...);
    }

    class_fn_impl &operator=(std::nullptr_t) noexcept {
        _fn = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        return _fn != nullptr;
    }
};

template <typename _T>
inline bool operator==(const class_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return !static_cast<bool>(x);
}

template <typename _T>
inline bool operator==(std::nullptr_t, const class_fn_impl<_T> &x) noexcept {
    return !static_cast<bool>(x);
}

template <typename _T>
inline bool operator!=(const class_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return static_cast<bool>(x);
}

template <typename _T>
inline bool operator!=(std::nullptr_t, const class_fn_impl<_T> &x) noexcept {
    return static_cast<bool>(x);
}

/* function_fn_impl */
template <typename _Sig>
class function_fn_impl;

template <typename _R, typename ..._Args>
class function_fn_impl<_R(_Args...)> {
public:
    typedef _R type(_Args...);
    typedef _R result_type;
private:
    type *_fn;

public:
    function_fn_impl(const function_fn_impl &x) noexcept
        : _fn(x._fn) {}
    function_fn_impl(function_fn_impl &&x) noexcept 
        : _fn() {
        std::swap(_fn, x._fn);
    }
    function_fn_impl(type fn) noexcept 
        : _fn(fn) {}

    result_type operator()(_Args...args) const noexcept {
        return (*_fn)(std::forward<_Args>(args)...);
    }

    result_type apply(_Args...args) const noexcept {
        return (*_fn)(std::forward<_Args>(args)...);
    }

    function_fn_impl &operator=(std::nullptr_t) noexcept {
        _fn = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        return _fn != nullptr;
    }
};

template <typename _T>
inline bool operator==(const function_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return !static_cast<bool>(x);
};

template <typename _T>
inline bool operator==(std::nullptr_t, const function_fn_impl<_T> &x) noexcept {
    return !static_cast<bool>(x);
};

template <typename _T>
inline bool operator!=(const function_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return static_cast<bool>(x);
};

template <typename _T>
inline bool operator!=(std::nullptr_t, const function_fn_impl<_T> &x) noexcept {
    return static_cast<bool>(x);
};

/* member_function_fn_impl */
template <typename _T>
class member_function_fn_impl;

template <typename _T, typename _R, typename ..._Args>
class member_function_fn_impl<_R(_T::*)(_Args...)> {
public:
    typedef _R(_T::*type)(_Args...);
    typedef _T class_type;
    typedef _R result_type;
protected:
    type _fn;

public:
    member_function_fn_impl(const member_function_fn_impl &x) noexcept 
        : _fn(x._fn) {}
    member_function_fn_impl(member_function_fn_impl &&x) noexcept
        : _fn() {
        std::swap(_fn, x._fn);
    }
    member_function_fn_impl(type fn) noexcept 
        : _fn(fn) {}

    result_type operator()(class_type *obj, _Args...args) const noexcept {
        return (obj->*_fn)(std::forward<_Args>(args)...);
    }

    result_type operator()(class_type &obj, _Args...args) const noexcept {
        return (obj.*_fn)(std::forward<_Args>(args)...);
    }

    result_type apply(class_type *obj, _Args...args) const noexcept {
        return (obj->*_fn)(std::forward<_Args>(args)...);
    }

    result_type apply(class_type &obj, _Args...args) const noexcept {
        return (obj.*_fn)(std::forward<_Args>(args)...);
    }

    member_function_fn_impl &operator=(std::nullptr_t) noexcept {
        _fn = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        return _fn != nullptr;
    }
};

template <typename _T, typename _R, typename ..._Args>
class member_function_fn_impl<_R(_T::*)(_Args...) const> {
public:
    typedef _R(_T::*type)(_Args...) const;
    typedef _T class_type;
    typedef _R result_type;
protected:
    type _fn;

public:
    member_function_fn_impl(type fn) noexcept 
        : _fn(fn) {}

    result_type operator()(const class_type *obj, _Args...args) const noexcept {
        return (obj->*_fn)(std::forward<_Args>(args)...);
    }

    result_type operator()(const class_type &obj, _Args...args) const noexcept {
        return (obj.*_fn)(std::forward<_Args>(args)...);
    }

    result_type apply(const class_type *obj, _Args...args) const noexcept {
        return (obj->*_fn)(std::forward<_Args>(args)...);
    }

    result_type apply(const class_type &obj, _Args...args) const noexcept {
        return (obj.*_fn)(std::forward<_Args>(args)...);
    }

    member_function_fn_impl &operator=(std::nullptr_t) noexcept {
        _fn = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        return _fn != nullptr;
    }
};

template <typename _T>
inline bool operator==(const member_function_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return !static_cast<bool>(x);
};

template <typename _T>
inline bool operator==(std::nullptr_t, const member_function_fn_impl<_T> &x) noexcept {
    return !static_cast<bool>(x);
};

template <typename _T>
inline bool operator!=(const member_function_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return static_cast<bool>(x);
};

template <typename _T>
inline bool operator!=(std::nullptr_t, const member_function_fn_impl<_T> &x) noexcept {
    return static_cast<bool>(x);
};

/* move_copy_fn_impl */
template <typename _T>
class move_copy_fn_impl {
public:
    typedef _T type;
protected:
    type _fn;
    bool _empty;

public:
    move_copy_fn_impl(const move_copy_fn_impl&) = delete;
    move_copy_fn_impl(move_copy_fn_impl &&x) noexcept 
        : _fn(std::move(x._fn)), _empty(true) {
        std::swap(_empty, x._empty);
    }
    move_copy_fn_impl(type &&fn) noexcept 
        : _fn(std::move(fn)), _empty(false) {}

    template <typename ..._Args>
    auto operator()(_Args&&...args) noexcept 
        -> decltype(std::declval<type>()(std::forward<_Args>(args)...)) {
        return _fn(std::forward<_Args>(args)...);
    }

    template <typename ..._Args>
    auto apply(_Args&&...args) noexcept 
        -> decltype(std::declval<type>()(std::forward<_Args>(args)...)) {
        return _fn(std::forward<_Args>(args)...);
    }

    move_copy_fn_impl &operator=(std::nullptr_t) noexcept {
        _empty = true;
        return *this;
    }

    explicit operator bool() const noexcept {
        return !_empty;
    }
};

template <typename _T>
inline bool operator==(const move_copy_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return !static_cast<bool>(x);
};

template <typename _T>
inline bool operator==(std::nullptr_t, const move_copy_fn_impl<_T> &x) noexcept {
    return !static_cast<bool>(x);
};

template <typename _T>
inline bool operator!=(const move_copy_fn_impl<_T> &x, std::nullptr_t) noexcept {
    return static_cast<bool>(x);
};

template <typename _T>
inline bool operator!=(std::nullptr_t, const move_copy_fn_impl<_T> &x) noexcept {
    return static_cast<bool>(x);
};

/* check fn type */
template <typename _T>
struct is_class_fn {
    typedef typename std::remove_reference<_T>::type        noref_type;
    typedef typename std::remove_pointer<noref_type>::type  type;
    typedef class_fn_impl<type>                             fn_type;

    static constexpr bool value = 
        std::is_class<type>::value &&
        (std::is_lvalue_reference<_T>::value ||
         std::is_pointer<noref_type>::value);
};

template <typename _T>
struct is_function_fn {
    typedef typename std::remove_reference<_T>::type        noref_type;
    typedef typename std::remove_pointer<noref_type>::type  type;
    typedef function_fn_impl<type>                          fn_type;

    static constexpr bool value = std::is_function<type>::value;
};

template <typename _T>
struct is_member_function_fn {
    typedef typename std::remove_reference<_T>::type        type;
    typedef member_function_fn_impl<type>                   fn_type;

    static constexpr bool value = std::is_member_pointer<type>::value;
};

template <typename _T>
struct is_move_copy_fn {
    typedef typename std::remove_reference<_T>::type        type;
    typedef move_copy_fn_impl<type>                         fn_type;

    static constexpr bool value = 
        std::is_class<type>::value &&
        std::is_rvalue_reference<_T>::value &&
        std::is_move_constructible<type>::value;
};

template <typename _T>
struct is_fn {
    static constexpr bool value = 
        is_class_fn<_T>::value ||
        is_function_fn<_T>::value ||
        is_member_function_fn<_T>::value ||
        is_move_copy_fn<_T>::value;
};

/* make_fn */
template <typename _T>
inline auto make_fn(_T &&fn) noexcept -> 
typename std::enable_if<
        is_class_fn<decltype(fn)>::value,
        typename is_class_fn<decltype(fn)>::fn_type
    >::type
{
    return typename is_class_fn<decltype(fn)>::fn_type(std::forward<_T>(fn));
}

template <typename _T>
inline auto make_fn(_T &&fn) noexcept -> 
typename std::enable_if<
        is_function_fn<decltype(fn)>::value,
        typename is_function_fn<decltype(fn)>::fn_type
    >::type
{
    return typename is_function_fn<decltype(fn)>::fn_type(std::forward<_T>(fn));
}

template <typename _T>
inline auto make_fn(_T &&fn) noexcept -> 
typename std::enable_if<
        is_member_function_fn<decltype(fn)>::value,
        typename is_member_function_fn<decltype(fn)>::fn_type
    >::type
{
    return typename is_member_function_fn<decltype(fn)>::fn_type(std::forward<_T>(fn));
}

template <typename _T>
inline auto make_fn(_T &&fn) noexcept -> 
typename std::enable_if<
        is_move_copy_fn<decltype(fn)>::value,
        typename is_move_copy_fn<decltype(fn)>::fn_type
    >::type
{
    return typename is_move_copy_fn<decltype(fn)>::fn_type(std::forward<_T>(fn));
}

template <typename _T>
inline auto make_fn(_T &&fn) noexcept -> 
typename std::enable_if<
        !is_fn<decltype(fn)>::value,
        void
    >::type
{
    static_assert(is_fn<decltype(fn)>::value,
                  "functor must is a function or a class member function or "
                  "a class operator functor or a lambda.");
}

/* functor */
template <typename _Sig, typename _F, typename ..._Params>
class functor_impl;

template <typename _F, typename ..._Args, typename ..._Params>
class functor_impl<void(_Args...), _F, _Params...> {
private:
    _F _fn;
    std::tuple<_Params...> _params;
public:
    typedef void signature(_Args...);
    typedef void result_type;

    functor_impl(functor_impl &&x) noexcept : 
        _fn(std::move(x._fn)), 
        _params(std::move(x._params)) {}

    template <typename _T>
    functor_impl(_T &&fn, _Params&&...params) noexcept :
        _fn(std::forward<_T>(fn)),
        _params(std::forward<_Params>(params)...) {}

    result_type operator()(_Args...args) noexcept {
        tuple_apply(_fn, _params, std::forward<_Args>(args)...);
    }

    result_type apply(_Args...args) noexcept {
        tuple_apply(_fn, _params, std::forward<_Args>(args)...);
    }

    functor_impl &operator=(std::nullptr_t) noexcept {
        _fn = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(_fn);
    }
};

template <typename _F, typename _R, typename ..._Args, typename ..._Params>
class functor_impl<_R(_Args...), _F, _Params...> {
private:
    _F _fn;
    std::tuple<_Params...> _params;
public:
    typedef _R signature(_Args...);
    typedef _R result_type;

    functor_impl(functor_impl &&x) noexcept : 
        _fn(std::move(x._fn)), 
        _params(std::move(x._params)) {}

    template <typename _T>
    functor_impl(_T &&fn, _Params&&...params) noexcept :
        _fn(std::forward<_T>(fn)),
        _params(std::forward<_Params>(params)...) {}

    result_type operator()(_Args...args) noexcept {
        return tuple_apply(_fn, _params, std::forward<_Args>(args)...);
    }

    result_type apply(_Args...args) noexcept {
        return tuple_apply(_fn, _params, std::forward<_Args>(args)...);
    }

    functor_impl &operator=(std::nullptr_t) noexcept {
        _fn = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(_fn);
    }
};

template <typename _Sig, typename _Base, typename ..._Params>
inline bool operator==(const functor_impl<_Sig, _Base, _Params...> &x, std::nullptr_t) noexcept {
    return !static_cast<bool>(x);
};

template <typename _Sig, typename _Base, typename ..._Params>
inline bool operator==(std::nullptr_t, const functor_impl<_Sig, _Base, _Params...> &x) noexcept {
    return !static_cast<bool>(x);
};

template <typename _Sig, typename _Base, typename ..._Params>
inline bool operator!=(const functor_impl<_Sig, _Base, _Params...> &x, std::nullptr_t) noexcept {
    return static_cast<bool>(x);
};

template <typename _Sig, typename _Base, typename ..._Params>
inline bool operator!=(std::nullptr_t, const functor_impl<_Sig, _Base, _Params...> &x) noexcept {
    return static_cast<bool>(x);
};

/* make_functor */
template <typename _Sig, typename _T, typename ..._Params>
inline auto make_functor(_T &&fn, _Params&&...params) noexcept -> 
    functor_impl<_Sig, decltype(make_fn(std::forward<_T>(fn))), _Params...> 
{
    typedef decltype(make_fn(std::forward<_T>(fn))) fn_type;
    return functor_impl<_Sig, fn_type, _Params...>(std::forward<_T>(fn), std::forward<_Params>(params)...);
}

} // namespace ll end

#endif
