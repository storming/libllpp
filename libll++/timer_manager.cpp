#include "timer_manager.h"
#include "log.h"
namespace ll {

/* timer_manager */
void timer_manager::modify_i(timer *timer, timeval expires) noexcept
{
    if (timer->_is_idle) {
        return;
    }

    if (timer == _cur) {
        timer->_expires = expires;
    }
    else {
        if (timer->_expires == expires) {
            return;
        } 

        _map.remove(timer);
        timer->_expires = expires;
        _map.insert(timer);
    }
}

void timer_manager::remove(timer *timer) noexcept
{
    if (_cur == timer) {
        return;
    }
    if (timer->_is_idle) {
        _list.remove(timer);
    }
    else {
        _map.remove(timer);
    }
    _delete<ll::timer>(timer);
}

timeval timer_manager::loop_i(timeval curtime) noexcept
{
    timer *timer;
    timeval expires;

    while ((_cur = _list.pop_front())) {
        _cur->_idle_closure->apply();
    }

    while (1) {
        timer = _cur = _map.front();
        if (!timer) {
            expires = time::max;
            break;
        }
        if (curtime < timer->_expires) {
            expires = timer->_expires;
            break;
        }
        dispatch(timer, curtime);
    }

    while ((_cur = _list.pop_front())) {
        _cur->_idle_closure->apply();
    }

    return expires;
}

inline void timer_manager::dispatch(timer *timer, timeval curtime) noexcept
{
    _map.remove(timer);

    while (1) {
        timeval d = timer->_timer_closure->apply(*timer, curtime);
        if (d <= 0) {
            _delete<ll::timer>(timer);
            return;
        }
        timer->_expires += d;
        if (timer->_expires > curtime) {
            _map.insert(timer);
            return;
        }
    };
}

}

