#ifndef __LIBLLPP_SOCKET_H__
#define __LIBLLPP_SOCKET_H__

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "memory.h"
#include "reactor.h"
#include "timer_manager.h"
#include "slotsig.h"

namespace ll {

class hostinfo {
protected:
    const char *_host;
    const char *_port;
public:
    hostinfo() : _host(), _port() {}
    hostinfo(const char *host, const char *port) : _host(host), _port(port) {}
    
    int init(const char *uri, pool *pl) noexcept;

    const char *get_host() noexcept {
        return _host;
    }

    const char *get_port() noexcept {
        return _port;
    }

    unsigned get_portn() {
        return strtoul(_port, nullptr, 10);
    }

};

class address {
protected:
    struct sockaddr_in _addr;

public:
    static constexpr unsigned length() {
        return sizeof(sockaddr_in);
    }

    operator const struct sockaddr*() {
        return (struct sockaddr*)&_addr;
    }

    operator struct sockaddr*() {
        return (struct sockaddr*)&_addr;
    }

    operator const struct sockaddr_in*() {
        return &_addr;
    }

    operator struct sockaddr_in*() {
        return &_addr;
    }

    int resolve(const char *host, const char *port);
    int resolve(hostinfo *info) {
        return resolve(info->get_host(), info->get_port());
    }

    const char *get_host() {
        return inet_ntoa(_addr.sin_addr);
    }

    unsigned get_port() {
        return ntohs(_addr.sin_port);
    }
};

class addrinfo : public address, public hostinfo {
public:
    addrinfo() : address(), hostinfo() {}
    addrinfo(const char *host, const char *port) : address(), hostinfo(host, port) {}

    using hostinfo::init;

    int resolve() {
        return address::resolve(this);
    }

    const char *get_host() {
        if (_host) {
            return _host;
        }
        return address::get_host();
    }

    unsigned get_port() {
        if (_port) {
            return hostinfo::get_portn();
        }
        return address::get_port();
    }
};

class connector : signal<int(connector&, int, int), true> {
private:
    file_io _fd;
    reactor *_reactor;
    timer_manager *_timermgr;
    address _addr;
    timeval _timeout;
    timeval _interval;
    timeval _conntime;
    timer *_timer;
    bool _emitting;

    void close_timer();
    timeval timer_handler(timer &, timeval);
    void connect_ready();
    int connect_handler(file_io&, int);
    int do_connect();
    int do_emit(int, int) noexcept;
    static int callback(closure_type&, connector&, int, int);
public:
    connector() : _reactor(), _timermgr(), _timeout(), _interval() {}
    connector(address &addr, reactor *reactor, timer_manager *timermgr, timeval timeout, timeval interval);
    ~connector();

    address &get_addr() {
        return _addr;
    }

    void set_addr(address &addr) {
        _addr = addr;
    }

    reactor *get_reactor() {
        return _reactor;
    }

    void set_reactor(reactor *value) {
        _reactor = value;
    }

    timer_manager *get_timer_manager() {
        return _timermgr;
    }

    void set_timer_manager(timer_manager *value) {
        _timermgr = value;
    }

    timeval get_timeout() {
        return _timeout;
    }

    void set_timeout(timeval value) {
        _timeout = time_prec_msec::adjust(value);
    }

    timeval get_interval() {
        return _interval;
    }

    void set_interval(timeval value) {
        _interval = time_prec_msec::adjust(value);
    }

    timeval get_connect_time() {
        return _conntime;
    }

    bool connecting() {
        return _timer != nullptr || _fd.opened();
    }

    int connect();
    void close();

    template <typename _F, typename ..._Args>
    int connect(_F &&f, _Args&&...args) {
        signal<int(connector&, int, int), true>::connect(std::forward<_F>(f), std::forward<_Args>(args)...);
        return connect();
    }

    template <typename _F, typename ..._Args>
    static int connect_to(address &addr, reactor *reactor, timer_manager *timermgr, 
                          timeval timeout, timeval interval, _F &&f, _Args&&...args) {
        connector *c = _new<connector>(nullptr, addr, reactor, timermgr, timeout, interval);
        return c->connect(callback, make_closure<signature>(std::forward<_F>(f), std::forward<_Args>(args)...));
    }

};

class listener : public signal<int(listener&, int, int, address&), true> {
public:
    static constexpr unsigned default_backlog = 128;
private:
    file_io _fd;
    address _addr;
    reactor *_reactor;
    timer_manager *_timermgr;
    unsigned _backlog;
    bool _emitting;
    int accept_handler(file_io&, int);
    int do_emit(int, int, address&);
    int callback(closure_type &handler, listener&, int, int, address&);
public:
    listener() : _reactor(), _timermgr(), _backlog(default_backlog) {}
    listener(address &addr, reactor *reactor, timer_manager *timermgr, unsigned backlog = default_backlog);

    address &get_addr() {
        return _addr;
    }

    void set_addr(address &addr) {
        _addr = addr;
    }

    reactor *get_reactor() {
        return _reactor;
    }

    void set_reactor(reactor *value) {
        _reactor = value;
    }

    timer_manager *get_timer_manager() {
        return _timermgr;
    }

    void set_timer_manager(timer_manager *value) {
        _timermgr = value;
    }

    unsigned get_backlog() {
        return _backlog;
    }

    bool listening() {
        return _fd.opened();
    }

    void close();
    int listen();

    template <typename _F, typename ..._Args>
    int listen(_F &&f, _Args&&...args) {
        connect(std::forward<_F>(f), std::forward<_Args>(args)...);
        return listen();
    }

    template <typename _F, typename ..._Args>
    static int listen_at(address &addr, reactor *reactor, unsigned backlog, _F &&f, _Args&&...args) {
        listener *l = _new<listener>(nullptr, addr, reactor, backlog);
        return l->listen(callback, make_closure<signature>(std::forward<_F>(f), std::forward<_Args>(args)...));
    }
};

} // namespace ll end

#endif
