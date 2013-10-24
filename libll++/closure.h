#ifndef __LIBLLPP_CLOSURE_H__
#define __LIBLLPP_CLOSURE_H__

#include "functor.h"
#include "construct.h"

namespace ll { // namespace ll begin

template <typename _Sig>
class closure;

template <typename _R, typename ..._Args>
class closure<_R(_Args...)> {
public:
    typedef _R (*invoker_type)(closure*, _Args...);
    typedef size_t (*destructor_type)(closure*);
    typedef _R signature(_Args...);

private:
    invoker_type _invoker;
    destructor_type _destructor;
public:
    closure(const closure &x) noexcept 
        : _invoker(x._invoker), 
        _destructor(x._destructor) {}

    closure(closure &&x) noexcept 
        : _invoker(), _destructor() {
        std::swap(_invoker, x._invoker);
        std::swap(_destructor, x._destructor);
    }
    closure(invoker_type invoker, destructor_type destructor) noexcept 
        : _invoker(invoker), _destructor(destructor) {}

    _R operator()(_Args... args) noexcept {
        return _invoker(this, std::forward<_Args>(args)...);
    }

    _R apply(_Args... args) noexcept {
        return _invoker(this, std::forward<_Args>(args)...);
    }

    static size_t destruct(closure *c) noexcept {
        return c->_destructor(c);
    }

    template <typename _T, typename _Allocator, typename ..._Params>
    static closure *_new(_Allocator &&a, _T &&fn, _Params&&...params) noexcept;

    template <typename _T, typename ..._Params>
    static closure *_new(_T &&fn, _Params&&...params) noexcept;

    template <typename _Allocator>
    static void _delete(_Allocator &&a, closure *c) noexcept {
        mem_free(std::forward<_Allocator>(a), c, destruct(c));
    }

    static void _delete(closure *c) noexcept;
};

template <typename _Sig, typename _Functor>
class closure_impl;

template <typename _R, typename _Functor, typename ..._Args>
class closure_impl<_R(_Args...), _Functor> : public closure<_R(_Args...)> {
public:
    typedef closure<_R(_Args...)> closure_type;
    typedef _R result_type;
    typedef _R signature(_Args...);
private:
    _Functor _fn;

    static result_type invoker(closure_type *c, _Args...args) noexcept {
        return static_cast<closure_impl*>(c)->_fn.apply(std::forward<_Args>(args)...);
    }

    static unsigned destructor(closure_type *c) noexcept {
        destroy<closure_impl>(c);
        return sizeof(closure_impl);
    }
public:
    closure_impl(const closure_impl &x) noexcept 
        : closure_type(x), _fn(x._fn) {}
    closure_impl(closure_impl &&x) noexcept 
        : closure_type(std::move(x)), _fn(std::move(x._fn)) {}

    template <typename _T, typename ..._Params>
    closure_impl(_T &&fn, _Params&&...params) noexcept : 
        closure_type(invoker, destructor),
        _fn(std::forward<_T>(fn), std::forward<_Params>(params)...) {}

};

template <typename _Sig, typename _T, typename ..._Params>
inline auto make_closure(_T &&fn, _Params&&...params) ->
    closure_impl<_Sig, decltype(make_functor<_Sig>(std::forward<_T>(fn), std::forward<_Params>(params)...))>
{
    typedef decltype(make_functor<_Sig>(std::forward<_T>(fn), std::forward<_Params>(params)...)) functor_type;
    typedef closure_impl<_Sig, functor_type> type;

    return type(std::forward<_T>(fn), std::forward<_Params>(params)...);
}

template <typename _R, typename ..._Args> 
    template <typename _T, typename _Allocator, typename ..._Params>
    inline closure<_R(_Args...)>*
closure<_R(_Args...)>::_new(_Allocator &&a, _T &&fn, _Params&&...params) noexcept
{
    typedef _R signature(_Args...);
    typedef decltype(make_functor<signature>(std::forward<_T>(fn), std::forward<_Params>(params)...)) functor_type;
    typedef closure_impl<signature, functor_type> type;
    return construct<type>(
        mem_alloc(std::forward<_Allocator>(a), sizeof(type)), 
        std::forward<_T>(fn), 
        std::forward<_Params>(params)...);
}

}

#endif


