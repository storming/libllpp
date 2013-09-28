#ifndef __LIBLLPP_SLOTSIG_H__
#define __LIBLLPP_SLOTSIG_H__

#include "list.h"
#include "closure.h"
#include "memory.h"

namespace ll {

template <typename _Sig, bool __once, typename ..._Args>
struct slotsig_helper {

    typedef _Sig signature_t;
    typedef closure<signature_t> closure_t;

    class no_once_policy {
    public:
        struct slot_base {
            slot_base(closure_t *c) : _entry(nullptr), _closure(c) {}
            clist_entry _entry;
            closure_t *_closure;
        };
    
        template <typename _T, typename ..._Params>
        class slot : public slot_base, public closure_t::template instance<_T, _Params...> {
            template <typename ...> friend class no_once_policy;
        private:
            using slot_base::_entry;
        public:
            slot(_T &obj, _Params&&...args) : 
                slot_base(this),
                closure_t::template instance<_T, _Params...>(obj, std::forward<_Params>(args)...) {}
    
            slot(_T *obj, _Params&&...args) : 
                slot_base(this),
                closure_t::template instance<_T, _Params...>(obj, std::forward<_Params>(args)...) {}
    
            slot(_T &obj, typename closure<int(_Args...)>::template instance<_T, _Params...>::member_t member, _Params&&...args) : 
                slot_base(this),
                closure_t::template instance<_T, _Params...>(obj, member, std::forward<_Params>(args)...) {}
    
            slot(_T *obj, typename closure<int(_Args...)>::template instance<_T, _Params...>::member_t member, _Params&&...args) : 
                slot_base(this),
                closure_t::template instance<_T, _Params...>(obj, member, std::forward<_Params>(args)...) {}
    
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
        ll_list(slot_base, _entry) _list;
    
    public:
        no_once_policy() : _list() {}
    
        void connect(slot_base &s) {
            _list.push_back(&s);
        }
    
        void connect(slot_base *s) {
            _list.push_back(s);
        }
    
        bool connected() {
            return !_list.empty();
        }
    
        void disconnect_all() {
            slot_base *s;
            while ((s = _list.front())) {
                s.disconnect();
            }
        }
    
        template <typename ..._Args2>
        int emit(_Args2&&...args) {
            int n;
            auto end = _list.end();
            for (auto it = _list.begin(); it != end; ++it) {
                if ((n = it.pointer()->_closure->apply(std::forward<_Args2>(args)...)) < 0) {
                    return n;
                }
            }
            return 0;
        }
    
        template <typename ..._Args2>
        int remit(_Args2&&...args) {
            int n;
            auto end = _list.rend();
            for (auto it = _list.rbegin(); it != end; --it) {
                if ((n = it.pointer()->_closure->apply(std::forward<_Args2>(args)...)) < 0) {
                    return n;
                }
            }
            return 0;
        }
    };
    
    class once_policy {
    public:
        closure<int(_Args...)> *_closure;
    
    public:
        once_policy() : _closure(nullptr) {}
    
        int connect(closure_t *c) {
            if (_closure) {
                return e_busy;
            }
            _closure = c;
            return ok;
        }
    
        bool connected() {
            return _closure != nullptr;
        }
    
        void disconnect() {
            _closure = nullptr;
        }
    
        template <typename ..._Args2>
        int emit(_Args2&&...args) {
            if (_closure) {
                return (*_closure)(std::forward<_Args2>(args)...);
            }
            return 0;
        }
    };

    class signal : public std::conditional<__once, once_policy, no_once_policy>::type {
    public:
        typedef slotsig_helper::signature_t signature_t;
        typedef slotsig_helper::closure_t closure_t;
    };
};


template <typename signature, bool __once = false>
class signal;

template <bool __once, typename ..._Args>
class signal<int(_Args...), __once> : public slotsig_helper<int(_Args...), __once, _Args...>::signal {};

}

#endif
