#ifndef __LIBLLPP_SLOTSIG_H__
#define __LIBLLPP_SLOTSIG_H__

#include "list.h"
#include "closure.h"

namespace ll {

template <typename signature, bool __once = false>
class signal;

template <bool __once, typename ..._Args>
class signal<int(_Args...), __once> {
public:
    class slot {
        template <typename, bool> friend class signal;
    protected:
        clist_entry _entry;
        int (*_callback)(slot*, _Args...);

        slot(int (*callback)(slot*, _Args...)) : _entry(nullptr), _callback(callback) {}
        int operator()(_Args&&...args) {
            return _callback(this, std::forward<_Args>(args)...);
        }
    public:
        void disconnect() {
            _entry.remove();
            _entry._next = nullptr;
        }

        bool connected() {
            return _entry._next != nullptr;
        }
    };

    template <typename _F, typename ..._Params>
    class functor;

    template <typename _F, typename ..._Params>
    class functor : public slot, public closure<_F, _Params...> {
    private:
        typedef functor<_F, _Params...> self_t;
        typedef closure<_F, _Params...> closure_t;
        static int proxy(slot *s, _Args&&...args) {
            return static_cast<self_t*>(s)->apply(std::forward<_Args>(args)...);
        }
    public:
        explicit functor(_F &f, _Params&&...params) : slot(proxy), closure_t(f, std::forward<_Params>(params)...) {}
        explicit functor(_F *f, _Params&&...params) : slot(proxy), closure_t(f, std::forward<_Params>(params)...) {}
    };

    template <typename _F, typename ..._Params>
    class member;

    template <typename _T, typename ..._MemberArgs, typename ..._Params>
    class member<int (_T::*)(_MemberArgs...), _Params...> : 
            public slot, 
            public closure<int (_T::*)(_MemberArgs...), _Params...> {
    private:
        typedef member<int (_T::*)(_MemberArgs...), _Params...> self_t;
        typedef closure<int (_T::*)(_MemberArgs...), _Params...> closure_t;
        typedef int (_T::*member_t)(_MemberArgs...);

        static int proxy(slot *s, _Args&&...args) {
            return static_cast<self_t*>(s)->apply(std::forward<_Args>(args)...);
        }
    public:
        explicit member(_T &obj, member_t f, _Params&&...params) : slot(proxy), closure_t(obj, f, std::forward<_Params>(params)...) {}
        explicit member(_T *obj, member_t f, _Params&&...params) : slot(proxy), closure_t(obj, f, std::forward<_Params>(params)...) {}
    };

private:
    ll_list(slot, _entry) _list;

public:
    void connect(slot &s) {
        _list.push_back(&s);
    }

    void connect(slot *s) {
        _list.push_back(s);
    }

    int emit(_Args&&...args) {
        int n;
        auto end = _list.end();
        for (auto it = _list.begin(); it != end; ++it) {
            if ((n = (*it)(std::forward<_Args>(args)...)) < 0) {
                return n;
            }
        }
        return 0;
    }

    int remit(_Args&&...args) {
        int n;
        auto end = _list.rend();
        for (auto it = _list.rbegin(); it != end; --it) {
            if ((n = (*it)(std::forward<_Args>(args)...)) < 0) {
                return n;
            }
        }
        return 0;
    }

};

};


#endif
