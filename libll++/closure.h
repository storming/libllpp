#ifndef __LIBLLPP_CLOSURE_H__
#define __LIBLLPP_CLOSURE_H__

#include "tuple_apply.h"


namespace ll {      // namespace ll

template <typename _F, typename ..._Params>
class closure;

template <typename _F, typename ..._Params>
class closure {
private:
    _F& _f;
    std::tuple<_Params...> _params;
public:
    typedef _F functor;
    explicit closure(_F &f, _Params&&...params) : _f(f), _params(std::forward<_Params>(params)...) {}
    explicit closure(_F *f, _Params&&...params) : _f(*f), _params(std::forward<_Params>(params)...) {}

    template <bool __apply_back = false, typename ..._Args>
    auto operator ()(_Args&&...args) -> decltype(tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...)) {
        return tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...);
    }

    template <bool __apply_back = false, typename ..._Args>
    auto apply(_Args&&...args) -> decltype(tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...)) {
        return tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...);
    }
};

template <typename _T, typename _R, typename ..._MemberArgs, typename ..._Params>
class closure<_R (_T::*)(_MemberArgs...), _Params...> {
public:
    typedef _R (_T::*member)(_MemberArgs...);

    struct functor {
        functor(_T *obj, member f): _obj(obj), _f(f) {}
        _T *_obj;
        member _f;
        template <typename ..._Args>
        _R operator()(_Args&&...args) {
            return (_obj->*_f)(std::forward<_Args>(args)...);
        }
    };
private:
    functor _f;
    std::tuple<_Params...> _params;
public:
    explicit closure(_T &obj, member f, _Params&&...params) : _f(&obj, f), _params(std::forward<_Params>(params)...) {}
    explicit closure(_T *obj, member f, _Params&&...params) : _f(obj, f), _params(std::forward<_Params>(params)...) {}

    template <bool __apply_back = false, typename ..._Args>
    auto operator ()(_Args&&...args) -> decltype(tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...)) {
        return tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...);
    }

    template <bool __apply_back = false, typename ..._Args>
    auto apply(_Args&&...args) -> decltype(tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...)) {
        return tuple_apply::apply<__apply_back>(_f, _params, std::forward<_Args>(args)...);
    }
};

template <typename _F, typename ..._Params>
inline closure<_F, _Params...>
the_closure(_F &f, _Params&&...params) {
    return closure<_F, _Params...>(f, std::forward<_Params>(params)...);
}

template <typename _T, typename _R, typename ..._MemberArgs, typename ..._Params>
inline closure<_R(_T::*)(_MemberArgs...), _Params...>
the_closure(_T &obj, _R(_T::*func)(_MemberArgs...), _Params&&...params) {
    return closure<_R(_T::*)(_MemberArgs...), _Params...>(obj, func, std::forward<_Params>(params)...);
}

}; // namespace ll end

#endif


