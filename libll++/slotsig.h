#ifndef __LIBLLPP_SLOTSIG_H__
#define __LIBLLPP_SLOTSIG_H__

#include "list.h"
#include "closure.h"
#include "memory.h"

namespace ll {

namespace slotsig_helper {

template <typename ..._Args>
class no_once_policy {
public:
    class slot {
        template <typename ...> friend class no_once_policy;
    protected:
        clist_entry _entry;
        int (*_callback)(slot*, _Args...);
    public:
        slot(int (*callback)(slot*, _Args...)) : _entry(nullptr), _callback(callback) {}
        ~slot() {
            if (connected()) {
                disconnect();
            }
        }
        int operator()(_Args&&...args) {
            return _callback(this, std::forward<_Args>(args)...);
        }

        void disconnect() {
            assert(_entry._next);
            _entry.remove();
            _entry._next = nullptr;
        }

        bool connected() {
            return _entry._next != nullptr;
        }
    };
private:
    ll_list(slot, _entry) _list;

public:
    no_once_policy() : _list() {}

    slot *connect(slot &s) {
        _list.push_back(&s);
        return &s;
    }

    slot *connect(slot *s) {
        _list.push_back(s);
        return s;
    }

    bool connected() {
        return !_list.empty();
    }

    bool connected(slot *s) {
        assert(s);
        auto end = _list.end();
        for (auto it = _list.begin(); it != end; ++it) {
            if (it->pointer() == s) {
                return true;
            }
        }
        return false;
    }

    void disconnect_all() {
        slot *s;
        while ((s = _list.front())) {
            s.disconnect();
        }
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

template <typename ..._Args>
class once_policy {
public:
    class slot {
        template <typename ...> friend class no_once_policy;
    protected:
        void **_entry;
        int (*_callback)(slot*, _Args...);
    public:
        slot(int (*callback)(slot*, _Args...)) : _entry(nullptr), _callback(callback) {}
        ~slot() {
            if (connected()) {
                disconnect();
            }
        }
        int operator()(_Args&&...args) {
            return _callback(this, std::forward<_Args>(args)...);
        }

        void disconnect() {
            assert(_entry);
            *_entry = nullptr;
            _entry = nullptr;
        }

        bool connected() {
            return _entry != nullptr;
        }
    };
private:
    slot *_slot;

public:
    once_policy() : _slot() {}

    bool connect(slot &s) {
        if (_slot) {
            return false;
        }
        _slot = &s;
        s._entry = &_slot;
        return true;
    }

    bool connect(slot *s) {
        if (_slot) {
            return false;
        }
        _slot = s;
        s->_entry = &_slot;
        return true;
    }

    bool connected() {
        return _slot != nullptr;
    }

    bool connected(slot *s) {
        assert(s);
        return _slot == s;
    }

    void disconnect_all() {
        if (_slot) {
            _slot.disconnect();
        }
    }

    int emit(_Args&&...args) {
        return (*_slot)(std::forward<_Args>(args)...);
    }

};

}

template <typename signature, bool __once = false>
class signal;

template <bool __once, typename ..._Args>
class signal<int(_Args...), __once> : public std::conditional<__once, slotsig_helper::once_policy<_Args...>, slotsig_helper::no_once_policy<_Args...>>::type {
public:
    typedef typename std::conditional<__once, slotsig_helper::once_policy<_Args...>, slotsig_helper::no_once_policy<_Args...>>::type base_t;
    typedef typename base_t::slot slot;

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

public:

    template <typename _F, typename ..._Params>
    static slot *create_slot(_F &f, _Params&&...params) {
        typedef functor<_F, _Params...> slot_t;
        return ll::create<slot_t>(f, std::forward<_Params>(params)...);
    }

    template <typename _F, typename ..._Params>
    static slot *create_slot(_F *f, _Params&&...params) {
        typedef functor<_F, _Params...> slot_t;
        return ll::create<slot_t>(f, std::forward<_Params>(params)...);
    }

    template <typename _T, typename ..._MemberArgs, typename ..._Params>
    static slot *create_slot(_T &f, int (_T::*mem_func)(_MemberArgs...), _Params&&...params) {
        typedef member<int (_T::*)(_MemberArgs...), _Params...> slot_t;
        return ll::create<slot_t>(f, mem_func, std::forward<_Params>(params)...);
    }

    template <typename _T, typename ..._MemberArgs, typename ..._Params>
    static slot *create_slot(_T *f, int (_T::*mem_func)(_MemberArgs...), _Params&&...params) {
        typedef member<int (_T::*)(_MemberArgs...), _Params...> slot_t;
        return ll::create<slot_t>(f, mem_func, std::forward<_Params>(params)...);
    }

    template <typename _F, typename _Allocator = pool, typename ..._Params>
    static slot *create_slot(_Allocator *a, _F &f, _Params&&...params) {
        typedef functor<_F, _Params...> slot_t;
        return ll::create<slot_t, factory<slot_t, _Allocator>>(a, f, std::forward<_Params>(params)...);
    }

    template <typename _F, typename _Allocator = pool, typename ..._Params>
    static slot *create_slot(_Allocator *a, _F *f, _Params&&...params) {
        typedef functor<_F, _Params...> slot_t;
        return ll::create<slot_t, factory<slot_t, _Allocator>>(a, f, std::forward<_Params>(params)...);
    }

    template <typename _T, typename _Allocator = pool, typename ..._MemberArgs, typename ..._Params>
    static slot *create_slot(_Allocator *a, _T &f, int (_T::*mem_func)(_MemberArgs...), _Params&&...params) {
        typedef member<int (_T::*)(_MemberArgs...), _Params...> slot_t;
        return ll::create<slot_t, factory<slot_t, _Allocator>>(a, f, mem_func, std::forward<_Params>(params)...);
    }

    template <typename _T, typename _Allocator = pool, typename ..._MemberArgs, typename ..._Params>
    static slot *create_slot(_Allocator *a, _T *f, int (_T::*mem_func)(_MemberArgs...), _Params&&...params) {
        typedef member<int (_T::*)(_MemberArgs...), _Params...> slot_t;
        return ll::create<slot_t, factory<slot_t, _Allocator>>(a, f, mem_func, std::forward<_Params>(params)...);
    }

    inline void destroy_slot(slot *s) {
        ll::destroy<slot>(s);
    }
};

}

#define ll_functor_slot(sig, functor, ...) decltype(sig)::functor<decltype(functor), ##__VA_ARGS__>
#define ll_member_slot(sig, obj, member_func, ...) decltype(sig)::member<decltype(&obj::member_func), ##__VA_ARGS__>

#endif
