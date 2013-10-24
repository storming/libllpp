#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>

#include "memory.h"
#include "socket.h"
#include "rc.h"
#include "guard.h"

namespace ll {

/* class hostinfo */
int hostinfo::init(const char *uri, pool *pool) noexcept
{
    const char *s;
    char *endstr;
    const char *rsb;
    int v6_offset1 = 0;

    /* We expect hostinfo to point to the first character of
     * the hostname.  There must be a port, separated by a colon
     */
    if (*uri == '[') {
        if ((rsb = strchr(uri, ']')) == nullptr || *(rsb + 1) != ':') {
            return fail;
        }
        /* literal IPv6 address */
        s = rsb + 1;
        ++uri;
        v6_offset1 = 1;
    } 
    else {
        s = strchr(uri, ':');
    }

    if (s == nullptr) {
        return fail;
    }

    _host = pool->strdup(uri, s - uri - v6_offset1);
    ++s;
    _port = pool->strdup(s);

    if (*s != '\0') {
        unsigned port_n = strtoul(_port, &endstr, 10);
        if (*endstr == '\0') {
            return port_n;
        }
        /* Invalid characters after ':' found */
    }
    return fail;
}

/* class address */
int address::resolve(const char *host, const char *port)
{
    struct ::addrinfo hints, *ai;
    int error;

    if (host && !*host) {
        host = nullptr;
    }

    memset(&hints, 0, sizeof(struct ::addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    error = ::getaddrinfo(host, port, &hints, &ai);
    if (error != 0) {
        return fail;
    }

    if (!ai) {
        return fail;
    }

    memcpy(&_addr, ai->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(ai);

    return ok;
}

/* connector */
connector::connector(address &addr, reactor *reactor, timer_manager *timermgr, 
                     timeval timeout, timeval interval) 
    : signal<int(connector&, int, int), true>(), _fd()
{
    _addr = addr;
    _timer = nullptr;
    _timeout = time_prec_msec::adjust(timeout);
    _interval = time_prec_msec::adjust(interval);
    _timermgr = timermgr;
    _reactor = reactor;
    _emitting = false;
}

connector::~connector() {
    close();
}

inline int connector::do_emit(int fd, int type) noexcept
{
    _emitting = true;
    int n = emit(*this, fd, type);
    _emitting = false;
    return n;
}

inline void connector::close_timer() {
    if (_timer) {
        _timermgr->remove(_timer);
        _timer = nullptr;
    }
}

timeval connector::timer_handler(timer &, timeval) {
    _timer = nullptr;

    if (_fd.opened()) {
        if (ll_failed(do_emit(_fd, reactor::poll_err))) {
            close();
            return 0;
        }
        _reactor->close(_fd);
    }

    connect();
    return 0;
}

void connector::connect_ready()
{
    _timer = nullptr;
    int fd = _fd.deattch();
    _reactor->close(fd);
    do_emit(fd, reactor::poll_out);
}

int connector::connect_handler(file_io&, int type) {
    int n;
    socklen_t len;

    if (type & reactor::poll_close) {
        _fd.close();
        return fail;
    }

    if (type & (reactor::poll_out | reactor::poll_err)) {
        close_timer();

        len = sizeof(int);
        ll_sys_failed_return(getsockopt(_fd, SOL_SOCKET, SO_ERROR, &n, &len));

        if (n || (type & reactor::poll_err)) {
            if (ll_ok(do_emit(_fd, reactor::poll_err))) {
                if (_interval) {
                    _timer = _timermgr->schedule(_conntime + _interval, &connector::timer_handler, this);
                    return fail;
                }
            }
            return fail;
        } 
        else {
            _timer = _timermgr->idle(&connector::connect_ready, this);
            return ok;
        }
    }

    return ok;
}

int connector::do_connect() 
{
    int n;
    _conntime = time_prec_msec::now();

again:
    n = ::connect(_fd, _addr, _addr.length());
    if (ll_sys_failed(n)) {
        switch (errno) {
        case EINTR:
            goto again;
        case ECONNREFUSED: 
            ll_failed_return(do_emit(_fd, reactor::poll_err));
            if (_interval) {
                _reactor->close(_fd);
                _timer = _timermgr->schedule(_conntime + _interval, &connector::timer_handler, this);
                return ok;
            }
            return fail;
        case EINPROGRESS:
            if (_timeout) {
                _timer = _timermgr->schedule(_conntime + _timeout, &connector::timer_handler, this);
            }
            return ok;
        default:
            return fail;
        }
    }

    return ok;
}

int connector::connect()
{
    if (connecting()) {
        return e_busy;
    }

    _emitting = false;
    ll_sys_failed_return(_fd = ::socket(AF_INET, SOCK_STREAM, 0));

    if (ll_failed(_reactor->open(_fd, reactor::poll_out | reactor::poll_err, &connector::connect_handler, this))) {
        _fd.close();
        return fail;
    }

    if (ll_failed(emit(*this, _fd, reactor::poll_open)) || ll_failed(do_connect())) {
        _reactor->close(_fd);
        return fail;
    }

    return ok;
}

void connector::close()
{
    if (!_emitting) {
        if (_fd.opened()) {
            _reactor->close(_fd);
        }
        close_timer();
    }
}

int connector::callback(closure_type &handler, connector &c, int fd, int type)
{
    int n = handler(c, fd, type);
    if (ll_failed(n) || (type & reactor::poll_out)) {
        c._emitting = false;
        c._timermgr->idle([](connector *p){ _delete<connector>(nullptr, p); }, std::addressof(c));
    }
    return n;
}

/* listener */
listener::listener(address &addr, reactor *reactor, timer_manager *timermgr, unsigned backlog) : _fd()
{
    _addr = addr;
    _reactor = reactor;
    _timermgr = timermgr;
    _emitting = false;
    _backlog = backlog ? backlog : default_backlog;
}

inline int listener::do_emit(int fd, int flags, address &addr)
{
    _emitting = true;
    int n = emit(*this, fd, flags, addr);
    _emitting = false;
    return n;
}

int listener::accept_handler(file_io&, int type)
{
    if (type & reactor::poll_close) {
        do_emit(_fd, reactor::poll_close, _addr);
        close();
        return -1;
    }

    if (type & reactor::poll_err) {
        do_emit(_fd, reactor::poll_err, _addr);
        return -1;
    }

    if (type & reactor::poll_in) {
        address addr;
        while (1) {
            socklen_t len = address::length();
            int fd = ::accept(_fd, addr, &len);
    
            if (fd < 0) {
                switch (errno) {
                case EAGAIN:
                    return ok;
                case EINTR:
                    continue;
                default:
                    do_emit(_fd, reactor::poll_err, _addr);
                    return -1;
                }
            }

            ll_failed_return(do_emit(fd, reactor::poll_in, addr));
        }
    }
    return ok;
}

int listener::listen()
{
    if (listening()) {
        return e_busy;
    }

    ll_sys_failed_return(_fd = ::socket(AF_INET, SOCK_STREAM, 0));

    auto guard = make_guard([this](){ _fd.close(); });
    int n = 1;
    ll_sys_failed_return(::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int)));
    ll_sys_failed_return(::bind(_fd, _addr, _addr.length()));
    ll_sys_failed_return(::listen(_fd, _backlog));
    ll_failed_return(_reactor->open(_fd, reactor::poll_in | reactor::poll_err, &listener::accept_handler, this));
    guard.dismiss();

    if (ll_failed(do_emit(_fd, reactor::poll_open, _addr))) {
        close();
        return fail;
    }
    return ok;
}

void listener::close() 
{
    if (!_emitting) {
        if (_fd.opened()) {
            _reactor->close(_fd);
        }
    }
}

int listener::callback(closure_type &handler, listener &l, int fd, int type, address &addr)
{
    int n = handler(l, fd, type, addr);
    if (ll_failed(n)) {
        l._emitting = false;
        l._timermgr->idle([](listener *p){ _delete<listener>(nullptr, p); }, std::addressof(l));
    }
    return n;
}

} // namespace ll end
