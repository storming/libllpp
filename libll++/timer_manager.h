#ifndef __LIBLLPP_TIMER_MANAGER_H__
#define __LIBLLPP_TIMER_MANAGER_H__

#include "slotsig.h"
#include "timeval.h"
#include "map.h"
#include "memory.h"

namespace ll {

class timer : public signal<int(timer&, timeval), true> {
    friend class timer_manager_base;
private:
    map_entry _entry;
    timeval _expires;
    timeval _interval;
    unsigned _seq;
public:
    timer(timeval expires, timeval interval);
    static timeval get_key(timer *t);
};

class timer_manager_base {
private:
    ll_map(timeval, timer, _entry) _map;
    typed_cache<timer> _cache;
    unsigned _counter;
    void update_seq(timer *timer);
    void dispatch(timer *timer, timeval curtime);

protected:
    timer_manager_base(pool *pool);
    timer *schedule(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr);
    void modify(timer *timer, timeval expires, timeval interval = 0);
    void remove(timer *timer);
    timeval loop(timeval curtime);
};

template <typename _Precision>
class timer_manager_ex : public timer_manager_base {
public:
    timer_manager_ex(pool *pool) : timer_manager_base(pool) {}

    timer *schedule(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr) {
        return timer_manager_base::schedule(_Precision::adjust(expires), _Precision::adjust(interval), handler);
    }

    timer *schedule_r(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr) {
        return timer_manager_base::schedule(_Precision::now() + _Precision::adjust(expires), _Precision::adjust(interval), handler);
    }

    void modify(timer *timer, timeval expires, timeval interval = 0) {
        timer_manager_base::modify(timer, _Precision::adjust(expires), _Precision::adjust(interval));
    }

    void modify_r(timer *timer, timeval expires, timeval interval = 0) {
        timer_manager_base::modify(timer, _Precision::now() + _Precision::adjust(expires), _Precision::adjust(interval));
    }

    timeval loop() {
        timeval cur = _Precision::now();
        while (1) {
            timeval t = timer_manager_base::loop(cur);
            cur = _Precision::now();
            if (t > cur) {
                return cur - t;
            }
        }
    }
};

using timer_manager = timer_manager_ex<time_precision_msec>;

}

#endif
