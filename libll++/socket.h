#ifndef __LIBLLPP_SOCKET_H__
#define __LIBLLPP_SOCKET_H__

#include <netdb.h>
#include "memory.h"
#include "reactor.h"
#include "timer_manager.h"

namespace ll {
namespace socket {

class hostinfo {
protected:
    const char *_host;
    const char *_port;
public:
    hostinfo() : _host(), _port() {}
    hostinfo(const char *host, const char *port) : _host(host), _port(port) {}
    const char *host() {
        return _host;
    }
    const char *port() {
        return _port;
    }

    int parse(const char *info, pool *pool);
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

    operator const struct sockaddr_in*() {
        return &_addr;
    }

    int getinfo(const char *host, const char *port);
    int getinfo(hostinfo *info) {
        return getinfo(info->host(), info->port());
    }
};

int connect(address *addr, reactor *reactor, timer_manager *timermgr, 
            timeval timeout, timeval interval,
            closure<int(int, int, address&)> *handler);

} // namespace socket end
} // namespace ll end

#endif
