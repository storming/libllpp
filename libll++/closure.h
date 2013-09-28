#ifndef __LIBLLPP_CLOSURE_H__
#define __LIBLLPP_CLOSURE_H__

#include "tuple_apply.h"
#include "friend.h"
#include "memory.h"

namespace ll {      // namespace ll

template <typename signature>
class closure;

template <typename _R, typename ..._Args>
class closure<_R(_Args...)> {
private:
    typedef closure<_R(_Args...)> closure_t;
    typedef _R (*calllback_t)(closure_t*, _Args...);
    calllback_t _callback;
    closure(calllback_t callback) : _callback(callback) {}

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
public:
    template <typename _T, typename ..._Params>
    class instance : public closure_t {
    public:
        typedef instance<_T, _Params...> instance_t;
        typedef _R (_T::*member_t)(_Params..., _Args...);

    private:
        struct proxy {
            proxy(_T &obj) : _obj(obj) {}
            proxy(_T &obj, member_t member) : _obj(obj), _member(member) {}
            _T &_obj;
            member_t _member;

            template <typename ..._CallArgs>
            _R operator()(_CallArgs&&...args) {
                return (_obj.*_member)(std::forward<_CallArgs>(args)...);
            }
        };
        
        proxy _proxy;
        std::tuple<_Params...> _params;

        static _R functor_apply(closure_t *c, _Args...args) {
            instance_t *p = static_cast<instance_t*>(c);
            return tuple_apply::apply(p->_proxy._obj, p->_params, std::forward<_Args>(args)...);
        }

        static _R member_apply(closure_t *c, _Args...args) {
            instance_t *p = static_cast<instance_t*>(c);
            return tuple_apply::apply(p->_proxy, p->_params, std::forward<_Args>(args)...);
        }

    public:
        explicit instance(_T &obj, _Params&&...params) : 
            closure_t(functor_apply), 
            _proxy(obj), 
            _params(std::forward<_Params>(params)...) {}
        explicit instance(_T *obj, _Params&&...params) : 
            closure_t(functor_apply), 
            _proxy(*obj), 
            _params(std::forward<_Params>(params)...) {}
        explicit instance(_T &obj, member_t member, _Params&&...params) : 
            closure_t(member_apply), 
            _proxy(obj, member), 
            _params(std::forward<_Params>(params)...) {}
        explicit instance(_T *obj, member_t member, _Params&&...params) : 
            closure_t(member_apply), 
            _proxy(*obj, member), 
            _params(std::forward<_Params>(params)...) {}
    };

    template <typename _F, typename ..._Params>
    static instance<_F, _Params...> make(_F &f, _Params&&...args) {
        return instance<_F, _Params...>(f, std::forward<_Params>(args)...);
    }

    template <typename _F, typename ..._Params>
    static instance<_F, _Params...> make(_F *f, _Params&&...args) {
        return instance<_F, _Params...>(f, std::forward<_Params>(args)...);
    }

    template <typename _T, typename ..._Params>
    static instance<_T, _Params...> make(_T &obj, typename instance<_T, _Params...>::member_t f, _Params&&...args) {
        return instance<_T, _Params...>(obj, f, std::forward<_Params>(args)...);
    }

    template <typename _T, typename ..._Params>
    static instance<_T, _Params...> make(_T *obj, typename instance<_T, _Params...>::member_t f, _Params&&...args) {
        return instance<_T, _Params...>(obj, f, std::forward<_Params>(args)...);
    }

    template <typename _F, typename ..._Params>
    static instance<_F, _Params...> *create(_F &f, _Params&&...args) {
        return ll::create<instance<_F, _Params...>>(f, std::forward<_Params>(args)...);
    }

    template <typename _T, typename ..._Params>
    static instance<_T, _Params...> *make(_T &obj, typename instance<_T, _Params...>::member_t f, _Params&&...args) {
        return ll::create<instance<_T, _Params...>>(obj, f, std::forward<_Params>(args)...);
    }

};

} // namespace ll end

#endif


