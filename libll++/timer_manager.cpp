#include "timer_manager.h"

namespace ll {

/* timer */
inline timer::timer(timeval expires, timeval interval) : _expires(expires), _interval(interval) 
{
}

inline timeval timer::get_key(timer *t) 
{
    return t->_expires;
}

/* timer_manager */
timer_manager::timer_manager(pool *pool) : _cache(caches::instance()->get<timer>()), _counter()
{
}

inline void timer_manager::update_seq(timer *timer)
{
    if (++_counter == 0) {
        _counter = 1;
    }
    timer->_seq = _counter;
}

timer *timer_manager::schedule_i(timeval expires, timeval interval, timer::closure_t *handler)
{
    timer *timer = _new<ll::timer>(_cache, expires, interval);
    _map.insert(timer);
    if (handler) {
        timer->connect(handler);
    }
    update_seq(timer);
    return timer;
}

void timer_manager::modify_i(timer *timer, timeval expires, timeval interval)
{
    timer->_interval = interval;
    if (timer->_expires != expires) {
        _map.remove(timer);
        timer->_expires = expires;
        _map.insert(timer);
        update_seq(timer);
    }
}

void timer_manager::remove(timer *timer)
{
    _map.remove(timer);
    timer->disconnect();
    timer->_seq = 0;
    _cache->_delete<ll::timer>(timer);
}

timeval timer_manager::loop_i(timeval curtime)
{
    while (1) {
        timer *timer = _map.front();
        if (!timer) {
            return time::max;
        }
        if (curtime < timer->_expires) {
            return timer->_expires;
        }
        dispatch(timer, curtime);
    }
}

inline void timer_manager::dispatch(timer *timer, timeval curtime)
{
    unsigned seq = timer->_seq;
    int n = timer->emit(*timer, curtime);
    if (timer->_seq == seq) {
        if (ll_ok(n) && timer->_interval > 0) {
            modify(timer, timer->_expires + timer->_interval, timer->_interval);
        }
        else {
            remove(timer);
        }
    }
}

}

