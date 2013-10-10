#ifndef __LIBLLPP_REACTOR_H__
#define __LIBLLPP_REACTOR_H__

#include <cassert>

#include "memory.h"
#include "slotsig.h"
#include "file_io.h"
#include "timeval.h"

struct epoll_event;

namespace ll {

class reactor {
public:
    static constexpr unsigned poll_in    = (1 << 0);
    static constexpr unsigned poll_out   = (1 << 1);
    static constexpr unsigned poll_err   = (1 << 2);
    static constexpr unsigned poll_hup   = (1 << 3);
    static constexpr unsigned poll_open  = (1 << 4);
    static constexpr unsigned poll_close = (1 << 5);

    class io : public file_io, public signal<int(io&, int), mallocator, true> {
        friend class reactor;
    private:
        void deattch();
    public:
        io() : file_io(), signal<int(io&, int), mallocator, true>() {}
    };

private:
    static unsigned _default_maxfds;
    static unsigned _default_maxevents;

    pool *_pool;
    unsigned _maxfds;
    unsigned _maxevents;
    int _fd;
    io **_fds;
    pool::stub *_stub;
    struct ::epoll_event *_events;

    void dispose();
public:
    reactor(pool *apool, unsigned maxfds = 0, unsigned maxevents = 0);
    ~reactor();

    void set_default_params(unsigned maxfds, unsigned maxevents);

    io *get(int fd) {
        assert(ll_fd_valid(fd) && (unsigned)fd < _maxfds);
        return _fds[fd];
    }

    io *operator [](int fd) {
        assert(ll_fd_valid(fd) && (unsigned)fd < _maxfds);
        return _fds[fd];
    }

    unsigned maxfds() {
        return _maxfds;
    }

    unsigned maxevents() {
        return _maxevents;
    }

    int open(int fd, unsigned flags);

    template <typename _F, typename ..._Args>
    int open(int fd, unsigned flags, _F &&f, _Args&&...args) {
        ll_failed_return(open(fd, flags));
        _fds[fd]->connect(std::forward<_F>(f), std::forward<_Args>(args)...);
    }

    int close(int fd, bool linger = false);
    int modify(int fd, int flags);
    int loop(timeval tv);

    static reactor *instance() {
        static reactor inst(pool::global());
        return &inst;
    }
};

}
#endif

