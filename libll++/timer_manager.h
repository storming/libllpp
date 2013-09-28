#ifndef __LIBLLPP_TIMER_MANAGER_H__
#define __LIBLLPP_TIMER_MANAGER_H__

#include "slotsig.h"
#include "timeval.h"
#include "map.h"
#include "memory.h"

namespace ll {

class timer : public signal<int(timer&, timeval), true> {
    friend class timer_manager;
private:
    map_entry _entry;
    timeval _expires;
    timeval _interval;
    unsigned _seq;
public:
    timer(timeval expires, timeval interval);
    static timeval get_key(timer *t);
};

class timer_manager {
private:
    ll_map(timeval, timer, _entry) _map;
    typed_cache<timer> _cache;
    unsigned _counter;
    void update_seq(timer *timer);
    void dispatch(timer *timer, timeval curtime);

    timer *schedule_i(timeval expires, timeval interval, timer::closure_t *handler);
    void modify_i(timer *timer, timeval expires, timeval interval);
    timeval loop_i(timeval curtime);

public:
    timer_manager(pool *pool);

    template <typename _Precision = time_precision_msec>
    timer *schedule(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr) {
        return schedule_i(_Precision::adjust(expires), _Precision::adjust(interval), handler);
    }

    template <typename _Precision = time_precision_msec>
    timer *schedule_r(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr) {
        return schedule_i(_Precision::now() + _Precision::adjust(expires), _Precision::adjust(interval), handler);
    }

    template <typename _Precision = time_precision_msec>
    void modify(timer *timer, timeval expires, timeval interval = 0) {
        modify_i(timer, _Precision::adjust(expires), _Precision::adjust(interval));
    }

    template <typename _Precision = time_precision_msec>
    void modify_r(timer *timer, timeval expires, timeval interval = 0) {
        modify_i(timer, _Precision::now() + _Precision::adjust(expires), _Precision::adjust(interval));
    }

    template <typename _Precision = time_precision_msec>
    timeval loop() {
        timeval cur = _Precision::now();
        while (1) {
            timeval t = loop_i(cur);
            cur = _Precision::now();
            if (t > cur) {
                return cur - t;
            }
        }
    }

    void remove(timer *timer);
};

}

#endif
