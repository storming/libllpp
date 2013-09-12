#include <time.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "reactor.h"
#include "etc.h"
#include "rc.h"

namespace ll {

unsigned reactor::_default_maxfds = 1024;
unsigned reactor::_default_maxevents = 32;

inline void reactor::io::deattch() 
{
    file_io::deattch();
    disconnect();
}

reactor::reactor(pool *apool, unsigned maxfds, unsigned maxevents) :
    _pool(apool),
    _maxfds(maxfds ? maxfds : _default_maxfds),
    _maxevents(maxevents ? maxevents : _default_maxevents)
{
    _fds = (io**)_pool->calloc(sizeof(io*) * _maxfds);
    _events = (struct ::epoll_event*)_pool->alloc(sizeof(struct ::epoll_event) * _maxevents);

    _fd = epoll_create(_maxfds);
    if ((_fd = epoll_create(_maxfds)) < 0) {
        crit_error("epoll_create", errno);
    }
}

reactor::~reactor()
{
    if (_fd >= 0) {
        ::close(_fd);
    }
}

void reactor::set_default_params(unsigned maxfds, unsigned maxevents)
{
    assert(maxfds && maxevents);
    _default_maxfds = maxfds;
    _default_maxevents = maxevents;
}

int reactor::open(int fd, unsigned flags, io::closure_t *handler)
{
    if (!ll_fd_valid(fd) && (unsigned)fd >= _maxfds) {
        return e_inval;
    }

    io *io = _fds[fd];
    if (!io) {
        _fds[fd] = io = create<reactor::io>(_pool);
    }

    ll_failed_return(io->open(fd));
    ll_failed_return_ex(io->set_block(false), io->deattch());

    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLET;

    if (flags & poll_in) {
        event.events |= EPOLLIN;
    }

    if (flags & poll_out) {
        event.events |= EPOLLOUT;
    }

    if (flags & poll_err) {
        event.events |= EPOLLERR;
    }

    if (flags & poll_hup) {
#ifdef EPOLLRDHUP
        event.events |= EPOLLRDHUP;
#else
        event.events |= EPOLLIN;
#endif
    }

    ll_sys_failed_return_ex(epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &event), io->deattch());
    
    if (handler) {
        ll_failed_return_ex(io->connect(handler), io->deattch());
    }
    return ok;
}

int reactor::close(int fd, bool linger)
{
    if (!ll_fd_valid(fd) && (unsigned)fd >= _maxfds) {
        return e_inval;
    }

    io *io = _fds[fd];
    if (!io) {
        return e_inval;
    }

    ll_sys_failed_return(epoll_ctl(_fd, EPOLL_CTL_DEL, fd, nullptr));
    io->emit(*io, poll_close);
    io->deattch();
    return ok;
}

int reactor::modify(int fd, int flags) 
{
    if (!ll_fd_valid(fd) && (unsigned)fd >= _maxfds) {
        return e_inval;
    }
    
    io *io = _fds[fd];
    if (!io) {
        return e_inval;
    }
        
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLET;

    if (flags & poll_in) {
        event.events |= EPOLLIN;
    }

    if (flags & poll_out) {
        event.events |= EPOLLOUT;
    }

    if (flags & poll_err) {
        event.events |= EPOLLERR;
    }

    if (flags & poll_hup) {
#ifdef EPOLLRDHUP
        event.events |= EPOLLRDHUP;
#else
        event.events |= EPOLLIN;
#endif
    }

    ll_sys_failed_return(epoll_ctl(_fd, EPOLL_CTL_MOD, fd, &event));

    return ok;
}

int reactor::loop(timeval tv)
{
    int nfds, flags;
    struct epoll_event *event;
    io *io;

    nfds = epoll_wait(_fd, _events, _maxevents, time_precision_msec::timeval2value(tv));

    if (ll_unlikely(nfds == -1)) {
        if (ll_likely(errno == EINTR)) {
            return ok;
        }
        return ll_sys_rc(errno);
    }
    else if (ll_likely(nfds > 0)) {
        for (event = _events; nfds--; event++) {
            io = _fds[event->data.fd];
            if (io->connected()) {
                flags = 0;
                if (event->events & EPOLLIN) {
                    flags |= poll_in;
                }
                if (event->events & EPOLLOUT) {
                    flags |= poll_out;
                }
                if (event->events & EPOLLERR) {
                    flags |= poll_err;
                }
#ifdef EPOLLRDHUP
                if (event->events & EPOLLRDHUP) {
                    flags |= poll_hup;
                }
#endif
                if (ll_failed(io->emit(*io, flags))) {
                    close(event->data.fd);
                }
            }
        }
    }
    return ok;


}

}
