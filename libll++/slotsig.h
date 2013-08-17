#ifndef __LIBLLPP_SLOTSIG_H__
#define __LIBLLPP_SLOTSIG_H__

#include "list.h"
#include "closure.h"

namespace ll {

template <typename signature>
class signal;

template <typename _R, typename ..._Args>
class signal<_R(_Args...)> {
public:
    class slot {
        template <typename> friend class signal;
    protected:
        clist_entry _entry;
        _R (*_proxy)(slot*, _Args...);

        slot(_R (*proxy)(slot*, _Args...)) : _proxy(proxy) {}
        _R proxy_apply(_Args&&...args) {
            return _proxy(this, std::forward<_Args>(args)...);
        }
    };

    template <typename _F, typename ..._Params>
    class functor;

    template <typename _F, typename ..._Params>
    class functor : public slot, public closure<_F, _Params...> {
    private:
        typedef functor<_F, _Params...> self_t;
        typedef closure<_F, _Params...> closure_t;
        static _R proxy(slot *s, _Args&&...args) {
            return static_cast<self_t*>(s)->apply(std::forward<_Args>(args)...);
        }
    public:
        explicit functor(_F &f, _Params&&...params) : slot(proxy), closure_t(f, std::forward<_Params>(params)...) {}
        explicit functor(_F *f, _Params&&...params) : slot(proxy), closure_t(f, std::forward<_Params>(params)...) {}
    };

    template <typename _F, typename ..._Params>
    class member;

    template <typename _T, typename ..._MemberArgs, typename ..._Params>
    class member<_R (_T::*)(_MemberArgs...), _Params...> : 
            public slot, 
            public closure<_R (_T::*)(_MemberArgs...), _Params...> {
    private:
        typedef member<_R (_T::*)(_MemberArgs...), _Params...> self_t;
        typedef closure<_R (_T::*)(_MemberArgs...), _Params...> closure_t;
        typedef _R (_T::*member_t)(_MemberArgs...);

        static _R proxy(slot *s, _Args&&...args) {
            return static_cast<self_t*>(s)->apply(std::forward<_Args>(args)...);
        }
    public:
        explicit member(_T &obj, member_t f, _Params&&...params) : slot(proxy), closure_t(obj, f, std::forward<_Params>(params)...) {}
        explicit member(_T *obj, member_t f, _Params&&...params) : slot(proxy), closure_t(obj, f, std::forward<_Params>(params)...) {}
    };

private:
    ll_list(slot, _entry) _list;

public:
    bool connect(slot &s) {
        _list.push_back(&s);
    }

    bool connect(slot *s) {
        _list.push_back(s);
    }

    _R emit(_Args&&...args) {
        for (auto it = _list.begin(); it != _list.end(); ++it) {
            (*it).proxy_apply(std::forward<_Args>(args)...);
        }
    }
};

};


#endif
