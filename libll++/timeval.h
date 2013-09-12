#ifndef __LIBLLPP_TIME_H__
#define __LIBLLPP_TIME_H__

#include <sys/time.h>
#include <limits>

namespace ll {

typedef long long timeval;
class time {
protected:
    timeval _value;

public:
    static constexpr timeval msecs_of_second = 1000L;
    static constexpr timeval usecs_of_second = 1000000L;
    static constexpr timeval nsecs_of_second = 1000000000L;
    static constexpr timeval max = std::numeric_limits<timeval>::max();

    time(timeval value = 0) : _value(value) {}

    static timeval now() {
        struct ::timeval tv;
        gettimeofday(&tv, nullptr);
        return static_cast<timeval>(tv.tv_sec) * usecs_of_second + static_cast<timeval>(tv.tv_usec);
    }

    timeval value() {
        return _value;
    }

    operator timeval() const {
        return _value;
    }

    timeval get_msec() {
        return _value / (usecs_of_second / msecs_of_second);
    }

    void set_msec(timeval value) {
        _value = value * (usecs_of_second / msecs_of_second);
    }

    timeval operator +(time &other) {
        return _value + other._value;
    }

    timeval operator +(timeval &value) {
        return _value + value;
    }

    timeval operator -(time &other) {
        return _value - other._value;
    }

    timeval operator -(timeval &value) {
        return _value - value;
    }

    bool operator >(time &other) {
        return _value > other._value;
    }

    bool operator >(timeval &value) {
        return _value > value;
    }

    bool operator >=(time &other) {
        return _value >= other._value;
    }

    bool operator >=(timeval &value) {
        return _value >= value;
    }

    bool operator <(time &other) {
        return _value < other._value;
    }

    bool operator <(timeval &value) {
        return _value < value;
    }

    bool operator <=(time &other) {
        return _value <= other._value;
    }

    bool operator <=(timeval &value) {
        return _value <= value;
    }

    bool operator ==(time &other) {
        return _value == other._value;
    }

    bool operator ==(timeval &value) {
        return _value == value;
    }

    bool operator !=(time &other) {
        return _value != other._value;
    }

    bool operator !=(timeval &value) {
        return _value != value;
    }

};

struct time_precision_msec {
    static timeval adjust(timeval tv) {
        return tv / (time::usecs_of_second / time::msecs_of_second) * (time::usecs_of_second / time::msecs_of_second);
    }

    static timeval timeval2value(timeval tv) {
        return tv / (time::usecs_of_second / time::msecs_of_second);
    }

    static timeval value2timeval(timeval tv) {
        return tv * (time::usecs_of_second / time::msecs_of_second);
    }

    static timeval now() {
        return adjust(time::now());
    }
};

class time_trace : public time {
public:
    time_trace() : time(now()) {}

    timeval check() {
        timeval n = now();
        timeval v = _value;
        _value = n;
        return n - v;
    }

    void reset() {
        _value = now();
    }
};

};
#endif

