#include <time.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "reactor.h"
#include "etc.h"
#include "rc.h"
#include "log.h"

namespace ll {

unsigned reactor::_default_maxfds = default_maxfds;
unsigned reactor::_default_maxevents = default_maxevents;

inline void reactor::io::deattch() 
{
    file_io::deattch();
    disconnect();
}

reactor::reactor(unsigned maxfds, unsigned maxevents) noexcept
    : reactor(pool::global(), maxfds, maxevents)
{
}

reactor::reactor(pool *pool, unsigned maxfds, unsigned maxevents) noexcept :
    _pool(pool),
    _maxfds(maxfds ? (maxfds < minfds ? minfds : maxfds) : _default_maxfds),
    _maxevents(maxevents ? (maxevents < minevents ? minevents : maxevents) : _default_maxevents)
{
    _fds = (io**)_pool->calloc(sizeof(io*) * _maxfds);
    _events = (struct ::epoll_event*)_pool->alloc(sizeof(struct ::epoll_event) * _maxevents);

    _fd = epoll_create(_maxfds);
    if ((_fd = epoll_create(_maxfds)) < 0) {
        crit_error("epoll_create", errno);
    }

    _stub = _pool->connect([this](){ dispose(); });
}

reactor::~reactor() noexcept
{
    if (_stub) {
        _pool->disconnect(_stub);
    }
    dispose();
}

void reactor::dispose() 
{
    if (ll_fd_valid(_fd)) {
        file_io::close(_fd);
    }
}

void reactor::set_default_params(unsigned maxfds, unsigned maxevents)
{
    assert(maxfds && maxevents);
    if (maxfds < minfds) {
        maxfds = minfds;
    }
    if (maxevents < minevents) {
        maxevents = minevents;
    }
    _default_maxfds = maxfds;
    _default_maxevents = maxevents;
}

int reactor::open(int fd, unsigned flags)
{
    if (!ll_fd_valid(fd) && (unsigned)fd >= _maxfds) {
        return e_inval;
    }

    io *io = _fds[fd];
    if (!io) {
        _fds[fd] = io = _new<reactor::io>(_pool);
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
    int timeout = time_prec_msec::to_precval(tv);
    if (!timeout) {
        return ok;
    }
    nfds = epoll_wait(_fd, _events, _maxevents, timeout);

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
