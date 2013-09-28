#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>

#include "socket.h"
#include "rc.h"

namespace ll {
namespace socket {

/* class hostinfo */
int hostinfo::parse(const char *uri, pool *pool)
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
int address::getinfo(const char *host, const char *port)
{
    struct addrinfo hints, *ai;
    int error;

    if (host && !*host) {
        host = nullptr;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
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

/* class connect_herlper */
struct connect_helper {
    int _fd;
    reactor *_reactor;
    timer_manager *_timer_mgr;
    address _addr;
    timeval _timeout;
    timeval _interval;
    timeval _contime;
    timer *_timer;
    timer::closure_t::instance<connect_helper> _timer_handler;
    closure<int(int, int, address&)> *_handler;

    typedef factory<connect_helper, caches> factory_t;
    connect_helper() : _timer_handler(this, &connect_helper::timer_handler) {
    }

    ~connect_helper() {
        if (ll_fd_valid(_fd)) {
            file_io::close(_fd);
        }

        if (_timer) {
            _timer_mgr->remove(_timer);
            _timer = nullptr;
        }
    }

    int timer_handler(timer &, timeval) {
        return ok;
    }

    static connect_helper *create() {
        return ll::create<connect_helper>(caches::global());
    }

    static void destroy(connect_helper *h) {
        ll::destroy(h, caches::global());
    }

    int connect() {
        ll_failed_return(_handler->apply(_fd, reactor::poll_open, _addr));
        _contime = time_precision_msec::now();
        int n;
again:
        n = ::connect(_fd, _addr, _addr.length());
        if (ll_sys_failed(n)) {
            switch (errno) {
            case EINTR:
                goto again;
            case ECONNREFUSED: 
                /*
                timer::closure_t *handler = ll::create<timer::closure_t>(this, &connect_helper::timer_handler);
                ll_failed_return(_handler->apply(_fd, reactor::poll_err, _addr));
                _timer = _timer_mgr->schedule(_contime + _interval, 0, handler);*/
                break;
            case EINPROGRESS:
                break;
            default:
                return fail;
            }
        }

        return ok;
    }
};

int connect(address *addr, reactor *reactor, timer_manager *timermgr, 
            timeval timeout, timeval interval,
            closure<int(int, int, address&)> *handler)
{
    assert(addr && handler);

    connect_helper *h = connect_helper::create();
    h->_addr = *addr;
    h->_fd = -1;
    h->_timer = nullptr;
    h->_timeout = timeout;
    h->_interval = time_precision_msec::adjust(interval);
    h->_handler = handler;
    ll_sys_failed_return_ex(h->_fd = ::socket(AF_INET, SOCK_STREAM, 0), connect_helper::destroy(h));
    ll_sys_failed_return_ex(h->connect(), connect_helper::destroy(h));

    return h->_fd;
}



} // namespace socket end
} // namespace ll end
