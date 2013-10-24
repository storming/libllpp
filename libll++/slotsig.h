#ifndef __LIBLLPP_SLOTSIG_H__
#define __LIBLLPP_SLOTSIG_H__

#include "list.h"
#include "closure.h"
#include "allocator.h"

namespace ll {

template <typename signature, bool __once = false, typename _Allocator = allocator>
class signal;

/* normal signal with allocator */
template <typename _Allocator, typename _R, typename ..._Args>
class signal<_R(_Args...), false, _Allocator> {
public:
    static_assert(std::is_void<_R>::value || std::is_integral<_R>::value,
        "signal result must is 'void' or integral.");

    typedef _R signature(_Args...);
    typedef closure<signature> closure_type;

    class slot {
        friend class signal;
        clist_entry _entry;
        closure_type *_closure;
    public:
        slot() : _entry(nullptr) {}
    };

private:
    typedef _Allocator allocator_type;
    struct impl : public allocator_type {
        ll_list(slot, _entry) _list;

        impl() noexcept : allocator_type(), _list() {}
        impl(const allocator_type &a) noexcept : allocator_type(a), _list() {}
        impl(allocator_type &&a) noexcept : allocator_type(std::move(a)), _list() {}
    };

    impl _impl;
public:
    signal() noexcept : _impl() {}
    signal(const allocator_type &a) noexcept : _impl(a) {}
    signal(allocator_type &&a) noexcept : _impl(std::move(a)) {}
    ~signal() noexcept {
        disconnect_all();
    }

    template <typename _T, typename ..._Params>
    slot *connect(_T &&obj, _Params&&...args) noexcept {
        slot *s = _new<slot>(_impl);
        s->_closure = _new<closure_type>(_impl, std::forward<_T>(obj), std::forward<_Params>(args)...);
        _impl._list.push_back(s);
        return s;
    }

    bool connected() noexcept {
        return !_impl._list.empty();
    }

    void disconnect(slot *s) noexcept {
        _impl._list.remove(s);
        _delete<closure_type>(_impl, s->_closure);
        _delete<slot>(_impl, s);
    }

    void disconnect_all() noexcept {
        slot *s;
        while ((s = _impl._list.front())) {
            disconnect(s);
        }
    }

    /* emit result is void */
    template <typename _Ret = _R>
    typename std::enable_if<
            std::is_void<_Ret>::value, _Ret
        >::type
    emit(_Args...args) {
        auto end = _impl._list.end();
        for (auto it = _impl._list.begin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
    }

    template <typename _Ret = _R>
    typename std::enable_if<
            std::is_void<_Ret>::value, _R
        >::type
    emit_and_disconnect(_Args...args) {
        slot *s;
        while ((s = _impl._list.front())) {
            s->_closure->apply(std::forward<_Args>(args)...);
            disconnect(s);
        }
    }

    /* emit result is boolean */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
            std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
            _Ret
        >::type
    emit(_F f, _Args...args) {
        auto end = _impl._list.end();
        for (auto it = _impl._list.begin(); it != end; ++it) {
            if (!f(it.pointer()->_closure->apply(std::forward<_Args>(args)...))) {
                return false;
            }
        }
        return true;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
            std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
            _Ret
        >::type
    emit(_Args...args) {
        auto end = _impl._list.end();
        for (auto it = _impl._list.begin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
        return true;
    }

    /* emit else */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
            (!std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
             !std::is_void<_Ret>::value), 
            _Ret
        >::type
    emit(_F f, _Args...args) {
        _Ret n;
        auto end = _impl._list.end();
        for (auto it = _impl._list.begin(); it != end; ++it) {
            if ((n = f(it.pointer()->_closure->apply(std::forward<_Args>(args)...)))) {
                return n;
            }
        }
        return 0;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
            (!std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
             !std::is_void<_Ret>::value), 
            _Ret
        >::type
    emit(_Args...args) {
        return emit([](_R r){ return r < 0 ? r : 0; }, std::forward<_Args>(args)...);
    }

    /* remit result is void */
    template <typename _Ret = _R>
    typename std::enable_if<
            std::is_void<_Ret>::value, _Ret
        >::type
    remit(_Args...args) {
        auto end = _impl._list.rend();
        for (auto it = _impl._list.rbegin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
    }

    /* remit result is boolean */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
            std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
            _Ret
        >::type
    remit(_F f, _Args...args) {
        auto end = _impl._list.rend();
        for (auto it = _impl._list.rbegin(); it != end; ++it) {
            if (!f(it.pointer()->_closure->apply(std::forward<_Args>(args)...))) {
                return false;
            }
        }
        return true;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
            std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
            _Ret
        >::type
    remit(_Args...args) {
        auto end = _impl._list.rend();
        for (auto it = _impl._list.rbegin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
        return true;
    }

    /* remit else */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
            (!std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
             !std::is_void<_Ret>::value), 
            _Ret
        >::type
    remit(_F f, _Args...args) {
        _Ret n;
        auto end = _impl._list.rend();
        for (auto it = _impl._list.rbegin(); it != end; ++it) {
            if ((n = f(it.pointer()->_closure->apply(std::forward<_Args>(args)...)))) {
                return n;
            }
        }
        return 0;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
            (!std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
             !std::is_void<_Ret>::value), 
            _Ret
        >::type
    remit(_Args...args) {
        return remit([](_R r){ return r < 0 ? r : 0; }, std::forward<_Args>(args)...);
    }
};

/* normal signal without allocator */
template <typename _R, typename ..._Args>
class signal<_R(_Args...), false, void> {
public:
    static_assert(std::is_void<_R>::value || std::is_integral<_R>::value,
        "signal result must is 'void' or integral.");

    typedef _R signature(_Args...);
    typedef closure<signature> closure_type;

    class slot {
        friend class signal;
        clist_entry _entry;
        closure_type *_closure;

    public:
        closure_type *get_closure() {
            return _closure;
        }

        void set_closure(closure_type *closure) {
            _closure = closure;
        }

        slot() noexcept : _entry(nullptr), _closure() {}
        slot(closure_type *closure) noexcept : _entry(nullptr), _closure(closure) {}
    };

private:
    ll_list(slot, _entry) _list;

public:
    signal() noexcept : _list() {}
    ~signal() noexcept {
        disconnect_all();
    }

    slot *connect(slot *s) noexcept {
        _list.push_back(s);
        return s;
    }

    bool connected() noexcept {
        return !_list.empty();
    }

    void disconnect(slot *s) noexcept {
        _list.remove(s);
    }

    void disconnect_all() noexcept {
        _list.init();
    }

    /* emit result is void */
    template <typename _Ret = _R>
    typename std::enable_if<std::is_void<_Ret>::value, _Ret>::type
    emit(_Args...args) {
        auto end = _list.end();
        for (auto it = _list.begin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
    }

    /* emit result is boolean */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
        std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
        _Ret>::type
    emit(_F f, _Args...args) {
        auto end = _list.end();
        for (auto it = _list.begin(); it != end; ++it) {
            if (!f(it.pointer()->_closure->apply(std::forward<_Args>(args)...))) {
                return false;
            }
        }
        return true;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
        std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
        _Ret>::type
    emit(_Args...args) {
        auto end = _list.end();
        for (auto it = _list.begin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
        return true;
    }

    /* emit else */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
        !std::is_void<_Ret>::value, _Ret>::type
    emit(_F f, _Args...args) {
        _Ret n;
        auto end = _list.end();
        for (auto it = _list.begin(); it != end; ++it) {
            if ((n = f(it.pointer()->_closure->apply(std::forward<_Args>(args)...)))) {
                return n;
            }
        }
        return 0;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
        !std::is_void<_Ret>::value, _Ret>::type
    emit(_Args...args) {
        return emit([](_R r){ return r < 0 ? r : 0; }, std::forward<_Args>(args)...);
    }

    /* remit result is void */
    template <typename _Ret = _R>
    typename std::enable_if<std::is_void<_Ret>::value, _Ret>::type
    remit(_Args...args) {
        auto end = _list.rend();
        for (auto it = _list.rbegin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
    }

    /* remit result is boolean */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
        std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
        _Ret>::type
    remit(_F f, _Args...args) {
        auto end = _list.rend();
        for (auto it = _list.rbegin(); it != end; ++it) {
            if (!f(it.pointer()->_closure->apply(std::forward<_Args>(args)...))) {
                return false;
            }
        }
        return true;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
        std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
        _Ret>::type
    remit(_Args...args) {
        auto end = _list.rend();
        for (auto it = _list.rbegin(); it != end; ++it) {
            it.pointer()->_closure->apply(std::forward<_Args>(args)...);
        }
        return true;
    }

    /* remit else */
    template <typename _F, typename _Ret = _R>
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
        !std::is_void<_Ret>::value, _Ret>::type
    remit(_F f, _Args...args) {
        _Ret n;
        auto end = _list.rend();
        for (auto it = _list.rbegin(); it != end; ++it) {
            if ((n = f(it.pointer()->_closure->apply(std::forward<_Args>(args)...)))) {
                return n;
            }
        }
        return 0;
    }

    template <typename _Ret = _R>
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
        !std::is_void<_Ret>::value, _Ret>::type
    remit(_Args...args) {
        return remit([](_R r){ return r < 0 ? r : 0; }, std::forward<_Args>(args)...);
    }
};

/* once signal with allocator */
template <typename _Allocator, typename _R, typename ..._Args>
class signal<_R(_Args...), true, _Allocator> {
public:
    static_assert(std::is_void<_R>::value || std::is_integral<_R>::value,
        "signal result must is 'void' or integral.");

    typedef _R signature(_Args...);
    typedef closure<signature> closure_type;

private:
    typedef _Allocator allocator_type;

    struct impl : public allocator_type {
        impl() noexcept : allocator_type(), _closure() {}
        impl(const allocator_type &a) noexcept : allocator_type(a), _closure() {}
        impl(allocator_type &&a) noexcept : allocator_type(std::move(a)), _closure() {}

        closure_type *_closure;
    };

    impl _impl;
public:
    signal() noexcept : _impl() {}
    signal(const allocator_type &a) noexcept : _impl(a) {}
    signal(allocator_type &&a) noexcept : _impl(std::move(a)) {}
    ~signal() noexcept {
        disconnect();
    }

    template <typename _T, typename ..._Params>
    void connect(_T &&obj, _Params&&...args) noexcept {
        disconnect();
        _impl._closure = _new<closure_type>(_impl, std::forward<_T>(obj), std::forward<_Params>(args)...);
    }

    bool connected() noexcept {
        return _impl._closure != nullptr;
    }

    void disconnect() noexcept {
        if (_impl._closure) {
            _delete<closure_type>(_impl, _impl._closure);
            _impl._closure = nullptr;
        }
    }

    /* emit result is void */
    template <typename _Ret = _R>
    typename std::enable_if<std::is_void<_Ret>::value, _Ret>::type
    emit(_Args...args) {
        closure_type *c = _impl._closure;
        if (c) {
            c->apply(std::forward<_Args>(args)...);
        }
    }

    /* emit result is boolean */
    template <typename _Ret = _R>
    typename std::enable_if<
        std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
        _Ret>::type
    emit(_Args...args) {
        closure_type *c = _impl._closure;
        if (c) {
            return c->apply(std::forward<_Args>(args)...);
        }
        return true;
    }

    /* emit else */
    template <typename _Ret = _R>
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
        !std::is_void<_Ret>::value, _Ret>::type
    emit(_Args...args) {
        closure_type *c = _impl._closure;
        if (c) {
            return c->apply(std::forward<_Args>(args)...);
        }
        return 0;
    }
};

/* once signal without allocator */
template <typename _R, typename ..._Args>
class signal<_R(_Args...), true, void> {
public:
    static_assert(std::is_void<_R>::value || std::is_integral<_R>::value,
        "signal result must is 'void' or integral.");

    typedef _R signature(_Args...);
    typedef closure<signature> closure_type;

private:
    closure_type *_closure;
public:
    signal() noexcept : _closure() {}

    void connect(closure_type *c) noexcept {
        _closure = c;
    }

    bool connected() noexcept {
        return _closure != nullptr;
    }

    closure_type *disconnect() noexcept {
        closure_type *tmp = _closure;
        _closure = nullptr;
        return tmp;
    }

    /* emit result is void */
    template <typename _Ret = _R>
    typename std::enable_if<std::is_void<_Ret>::value, _Ret>::type
    emit(_Args...args) {
        if (_closure) {
            _closure->apply(std::forward<_Args>(args)...);
        }
    }

    /* emit result is boolean */
    template <typename _Ret = _R>
    typename std::enable_if<
        std::is_same<typename std::remove_cv<_Ret>::type, bool>::value, 
        _Ret>::type
    emit(_Args...args) {
        if (_closure) {
            return _closure->apply(std::forward<_Args>(args)...);
        }
        return true;
    }

    /* emit else */
    template <typename _Ret = _R>
    typename std::enable_if<
        !std::is_same<typename std::remove_cv<_Ret>::type, bool>::value && 
        !std::is_void<_Ret>::value, _Ret>::type
    emit(_Args...args) {
        if (_closure) {
            return _closure->apply(std::forward<_Args>(args)...);
        }
        return 0;
    }
};

}
#endif
