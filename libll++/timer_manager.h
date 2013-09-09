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

    timeval now() {
        return time::now();
    }
public:
    timer_manager(pool *pool);
    timer *schedule(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr);
    timer *schedule_r(timeval expires, timeval interval = 0, timer::closure_t *handler = nullptr) {
        return schedule(now() + expires, interval, handler);
    }
    void modify(timer *timer, timeval expires, timeval interval = 0);
    void modify_r(timer *timer, timeval expires, timeval interval = 0) {
        return modify(timer, now() + expires, interval);
    }
    void remove(timer *timer);

    timeval loop(timeval curtime = 0);
};


}

#endif
