#ifndef __LIBLLPP_CLOSURE_H__
#define __LIBLLPP_CLOSURE_H__

#include <type_traits>
#include "tuple_apply.h"
#include "construct.h"

namespace ll { // namespace ll begin

template <typename _Sig>
class closure;

template <typename _R, typename ..._Args>
class closure<_R(_Args...)> {
private:
    typedef _R (*calllback_t)(closure*, _Args...);
    typedef size_t (*destroy_t)(closure*);

    calllback_t _callback;
    destroy_t _destroy;

    closure() {}
    closure(calllback_t callback, destroy_t destroy) noexcept : _callback(callback), _destroy(destroy) {}
public:
    typedef _R(*signature_t)(_Args...);

    template <typename ..._Args2>
    _R operator()(_Args2 &&... args) {
        return _callback(this, std::forward<_Args2>(args)...);
    }

    template <typename ..._Args2>
    _R apply(_Args2 &&... args) {
        return _callback(this, std::forward<_Args2>(args)...);
    }

    static size_t destroy(closure *c) {
        return c->_destroy(c);
    }

private:
    template <typename _T>
    struct is_functor {
        static constexpr int value = std::is_class<_T>::value || std::is_function<_T>::value;
    };

    template <typename _T>
    struct is_member_functor {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef noref_type type;

        static constexpr int value = std::is_member_function_pointer<type>::value;
    };

    template <typename _T>
    struct is_pointer_functor {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef typename std::remove_pointer<noref_type>::type type;

        static constexpr int value = 
            std::is_pointer<noref_type>::value && 
            is_functor<type>::value;
    };

    template <typename _T>
    struct is_lref_functor {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef typename std::remove_pointer<noref_type>::type type;

        static constexpr int value = 
            std::is_lvalue_reference<_T>::value &&
            !std::is_pointer<noref_type>::value && 
            is_functor<type>::value;
    };

    template <typename _T>
    struct is_rref_functor {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef typename std::remove_pointer<noref_type>::type type;

        static constexpr int value = 
            std::is_rvalue_reference<_T>::value &&
            !std::is_pointer<noref_type>::value && 
            is_functor<type>::value;
    };

    template <typename _T>
    struct member_class {};

    template <typename _T, typename _U>
    struct member_class<_T _U::*> {
        typedef _U type;
    };

    template <typename _T, typename ..._Params>
    struct member_functor_impl : public closure {
        typedef typename std::remove_reference<_T>::type type;
        typedef typename member_class<typename std::remove_cv<type>::type>::type object_type;

        struct proxy {
            type _fn;
            object_type *_obj;

            proxy(type fn, object_type &obj) : _fn(fn), _obj(std::addressof(obj)) {}
            proxy(type fn, object_type *obj) : _fn(fn), _obj(obj) {}

            template <typename ..._Args2>
            _R operator()(_Args2&&...args) {
                return (_obj->*_fn)(std::forward<_Args2>(args)...);
            }
        };
        proxy _proxy;
        std::tuple<_Params...> _params;

        static _R do_apply(closure *c, _Args...args) {
            member_functor_impl *p = static_cast<member_functor_impl*>(c);
            return tuple_apply::apply(p->_proxy, p->_params, std::forward<_Args>(args)...);
        }

        static size_t do_destroy(closure *c) {
            ll::destroy<member_functor_impl>(c);
            return sizeof(member_functor_impl);
        }

        template <typename _Obj>
        member_functor_impl(type fn, _Obj &&obj, _Params&&...params) noexcept : 
            closure(do_apply, do_destroy), 
            _proxy(fn, std::forward<_Obj>(obj)), 
            _params(std::forward<_Params>(params)...) {
        }
    };

    template <typename _T, typename ..._Params>
    struct pointer_functor_impl : public closure {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef typename std::remove_pointer<noref_type>::type type;

        type *_fn;
        std::tuple<_Params...> _params;
        
        static _R do_apply(closure *c, _Args...args) {
            pointer_functor_impl *p = static_cast<pointer_functor_impl*>(c);
            return tuple_apply::apply(*p->_fn, p->_params, std::forward<_Args>(args)...);
        }

        static size_t do_destroy(closure *c) {
            ll::destroy<pointer_functor_impl>(c);
            return sizeof(pointer_functor_impl);
        }

        pointer_functor_impl(type *fn, _Params&&...params) noexcept : 
            closure(do_apply, do_destroy), 
            _fn(fn), 
            _params(std::forward<_Params>(params)...) {
        }
    };

    template <typename _T, typename ..._Params>
    struct lref_functor_impl : public closure {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef typename std::remove_pointer<noref_type>::type type;

        type &_fn;
        std::tuple<_Params...> _params;

        static _R do_apply(closure *c, _Args...args) {
            lref_functor_impl *p = static_cast<lref_functor_impl*>(c);
            return tuple_apply::apply(p->_fn, p->_params, std::forward<_Args>(args)...);
        }

        static size_t do_destroy(closure *c) {
            ll::destroy<lref_functor_impl>(c);
            return sizeof(lref_functor_impl);
        }

        lref_functor_impl(type &fn, _Params&&...params) noexcept : 
            closure(do_apply, do_destroy), 
            _fn(fn), 
            _params(std::forward<_Params>(params)...) {
        }
    };

    template <typename _T, typename ..._Params>
    struct rref_functor_impl : public closure {
        typedef typename std::remove_reference<_T>::type noref_type;
        typedef typename std::remove_pointer<noref_type>::type type;

        type _fn;
        std::tuple<_Params...> _params;

        static _R do_apply(closure *c, _Args...args) {
            rref_functor_impl *p = static_cast<rref_functor_impl*>(c);
            return tuple_apply::apply(p->_fn, p->_params, std::forward<_Args>(args)...);
        }

        static size_t do_destroy(closure *c) {
            ll::destroy<rref_functor_impl>(c);
            return sizeof(rref_functor_impl);
        }

        rref_functor_impl(type &&fn, _Params&&...params) noexcept : 
            closure(do_apply, do_destroy), 
            _fn(fn), 
            _params(std::forward<_Params>(params)...) {
        }
    };

public:
    /* make member functor */
    template <typename _T, typename _Obj, typename ..._Params>
    static auto make(_T &&fn, _Obj &&obj, _Params&&...args) noexcept -> 
        typename std::enable_if<is_member_functor<decltype(fn)>::value, member_functor_impl<decltype(fn), _Params...>>::type {
        return member_functor_impl<decltype(fn), _Params...>(
            std::forward<_T>(fn), 
            std::forward<_Obj>(obj), 
            std::forward<_Params>(args)...);
    }

    /* make functor pointer */
    template <typename _T, typename ..._Params>
    static auto make(_T &&fn, _Params&&...args) noexcept -> 
        typename std::enable_if<is_pointer_functor<decltype(fn)>::value, pointer_functor_impl<decltype(fn), _Params...>>::type {
        return pointer_functor_impl<decltype(fn), _Params...>(
            std::forward<_T>(fn), 
            std::forward<_Params>(args)...);
    }

    /* make lvalue_reference functor */
    template <typename _T, typename ..._Params>
    static auto make(_T &&fn, _Params&&...args) noexcept -> 
        typename std::enable_if<is_lref_functor<decltype(fn)>::value, lref_functor_impl<decltype(fn), _Params...>>::type {
        return lref_functor_impl<decltype(fn), _Params...>(
            std::forward<_T>(fn), 
            std::forward<_Params>(args)...);
    }

    /* make rvalue_reference functor */
    template <typename _T, typename ..._Params>
    static auto make(_T &&fn, _Params&&...args) noexcept -> 
        typename std::enable_if<is_rref_functor<decltype(fn)>::value, rref_functor_impl<decltype(fn), _Params...>>::type {
        return rref_functor_impl<decltype(fn), _Params...>(
            std::forward<_T>(fn), 
            std::forward<_Params>(args)...);
    }

    /* _new member functor */
    template <typename _T, typename _Allocator, typename _Obj, typename ..._Params>
    static auto _new(_Allocator &&a, _T &&fn, _Obj &&obj, _Params&&...args) noexcept -> 
        typename std::enable_if<is_member_functor<decltype(fn)>::value, member_functor_impl<decltype(fn), _Params...>*>::type {
        return ll::_new<member_functor_impl<decltype(fn), _Params...>>(
            std::forward<_Allocator>(a), 
            std::forward<_T>(fn), 
            std::forward<_Obj>(obj), 
            std::forward<_Params>(args)...);
    }

    /* _new functor pointer */
    template <typename _T, typename _Allocator, typename ..._Params>
    static auto _new(_Allocator &&a, _T &&fn, _Params&&...args) noexcept -> 
        typename std::enable_if<is_pointer_functor<decltype(fn)>::value, pointer_functor_impl<decltype(fn), _Params...>*>::type {
        return ll::_new<pointer_functor_impl<decltype(fn), _Params...>>(
            std::forward<_Allocator>(a), 
            std::forward<_T>(fn), 
            std::forward<_Params>(args)...);
    }

    /* _new lvalue_reference functor */
    template <typename _T, typename _Allocator, typename ..._Params>
    static auto _new(_Allocator &&a, _T &&fn, _Params&&...args) noexcept -> 
        typename std::enable_if<is_lref_functor<decltype(fn)>::value, lref_functor_impl<decltype(fn), _Params...>*>::type {
        return ll::_new<lref_functor_impl<decltype(fn), _Params...>>(
            std::forward<_Allocator>(a), 
            std::forward<_T>(fn), 
            std::forward<_Params>(args)...);
    }

    /* _new rvalue_reference functor */
    template <typename _T, typename _Allocator, typename ..._Params>
    static auto _new(_Allocator &&a, _T &&fn, _Params&&...args) noexcept -> 
        typename std::enable_if<is_rref_functor<decltype(fn)>::value, rref_functor_impl<decltype(fn), _Params...>*>::type {
        return ll::_new<rref_functor_impl<decltype(fn), _Params...>>(
            std::forward<_Allocator>(a), 
            std::forward<_T>(fn), 
            std::forward<_Params>(args)...);
    }

    template <typename _Allocator>
    static void _delete(_Allocator &&a, closure *c) {
        _free(std::forward<_Allocator>(a), c, destroy(c));
    }
};

} // namespace ll end

#endif


