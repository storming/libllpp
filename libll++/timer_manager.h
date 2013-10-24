#ifndef __LIBLLPP_TIMER_MANAGER_H__
#define __LIBLLPP_TIMER_MANAGER_H__

#include "memory.h"
#include "closure.h"
#include "timeval.h"
#include "map.h"

namespace ll {

class timer {
    friend class timer_manager;
private:
    bool _is_idle;
    union {
        struct {
            map_entry _map_entry;
            timeval _expires;
        };
        clist_entry _list_entry;
    };
    union {
        closure<timeval(timer&, timeval)> *_timer_closure;
        closure<void()> *_idle_closure;
    };
public:
    static timeval get_key(timer *t) noexcept {
        return t->_expires;
    }

    template <typename _F, typename ..._Args>
    static timer *_new(_F &&f, _Args&&...args) {
        timer *t = (timer*)mem_alloc(sizeof(timer));
        t->_is_idle = true;
        t->_idle_closure = ll::_new<closure<void()>>(
            nullptr, std::forward<_F>(f), std::forward<_Args>(args)...);
        return t;
    }

    template <typename _F, typename ..._Args>
    static timer *_new(timeval expires, _F &&f, _Args&&...args) {
        timer *t = (timer*)mem_alloc(sizeof(timer));
        t->_is_idle = false;
        t->_expires = expires;
        t->_timer_closure = ll::_new<closure<timeval(timer&, timeval)>>(
            nullptr, std::forward<_F>(f), std::forward<_Args>(args)...);
        return t;
    }

    static void _delete(timer *t) {
        if (t->_is_idle) {
            ll::_delete<closure<void()>>(nullptr, t->_idle_closure);
        }
        else {
            ll::_delete<closure<timeval(timer&, timeval)>>(nullptr, t->_timer_closure);
        }
        mem_free(t, sizeof(timer));
    }
};

class timer_manager {
private:
    ll_map(timeval, timer, _map_entry) _map;
    ll_list(timer, _list_entry) _list;
    timer *_cur;

    void dispatch(timer *timer, timeval curtime) noexcept;
    void modify_i(timer *timer, timeval expires) noexcept;
    timeval loop_i(timeval curtime) noexcept;

public:
    timer_manager() noexcept : _map(), _list(), _cur() {}

    template <typename _F, typename _Precision = default_time_precision, typename ..._Args>
    timer *schedule(timeval expires, _F &&f, _Args&&...args) noexcept {
        timer *timer = _new<ll::timer>(_Precision::adjust(expires), 
                                       std::forward<_F>(f), std::forward<_Args>(args)...);
        _map.insert(timer);
        return timer;
    }

    template <typename _F, typename _Precision = default_time_precision, typename ..._Args>
    timer *schedule_r(timeval expires, _F &&f, _Args&&...args) noexcept {
        timer *timer = _new<ll::timer>(_Precision::now() + _Precision::adjust(expires), 
                                       std::forward<_F>(f), std::forward<_Args>(args)...);
        _map.insert(timer);
        return timer;
    }

    template <typename _F, typename ..._Args>
    timer *idle(_F &&f, _Args&&...args) noexcept {
        timer *timer = _new<ll::timer>(std::forward<_F>(f), std::forward<_Args>(args)...);
        _list.push_back(timer);
        return timer;
    }

    template <typename _Precision = default_time_precision>
    void modify(timer *timer, timeval expires) noexcept {
        modify_i(timer, _Precision::adjust(expires));
    }

    template <typename _Precision = default_time_precision>
    timeval loop() noexcept {
        timeval cur = _Precision::now();
        while (1) {
            timeval t = loop_i(cur);
            cur = _Precision::now();
            if (t > cur) {
                return t - cur;
            }
        }
    }

    void remove(timer *timer) noexcept;
};

}

#endif
