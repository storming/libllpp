#ifndef __LIBLLPP_TIME_H__
#define __LIBLLPP_TIME_H__

#include <sys/time.h>

namespace ll {

typedef long long timeval;
class time {
protected:
    timeval _value;

public:
    static constexpr timeval msecs_of_second = 1000L;
    static constexpr timeval usecs_of_second = 1000000L;
    static constexpr timeval nsecs_of_second = 1000000000L;

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

};

#endif

