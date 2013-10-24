#include <iostream>

using std::cout;
using std::endl;

#include "libll++/socket.h"
#include "libll++/timeval.h"

struct network {
    ll::reactor reactor;
    ll::timer_manager timermgr;
    ll::addrinfo addr;

    network() : reactor(), timermgr(){}

    int init() {
        ll_failed_return(addr.init("www.baidu.com:80", ll::pool::global()));
        ll_failed_return(addr.resolve());
        return ll::ok;
    }
};
network network;

int connect_handler(ll::connector&, int fd, int type);

int io_handler(ll::file_io &fd, int type) 
{
    if (type & ll::reactor::poll_close) {
        cout << (int)fd << " closed." << endl;
        fd.close();
        network.timermgr.idle(
            []() {
                cout << "reconnect" << endl;
                ll::connector::connect_to(
                    network.addr, &network.reactor, &network.timermgr, 
                    ll::time_prec_msec::to_timeval(2000),
                    ll::time_prec_msec::to_timeval(2000), connect_handler);
            });
        return -1;
    }
    return 0;
};

int connect_handler(ll::connector&, int fd, int type)
{
    if (type & ll::reactor::poll_open) {
        cout << "connect to" << endl;
    }

    if (type & ll::reactor::poll_err) {
        cout << "connect err" << endl;
    }

    if (type & ll::reactor::poll_out) {
        cout << "connect ok " << fd << endl;
        network.reactor.open(fd, ll::reactor::poll_in, io_handler);
        network.timermgr.schedule_r(
            ll::time_prec_msec::to_timeval(5000),
            [](int fd, ll::timer&, ll::timeval) {
                cout << "timer to close " << fd << endl;
                network.reactor.close(fd);
                return 0;
            }, std::move(fd));
    }
    return 0;
}

int main()
{
    ll_failed_return(network.init());
    ll_failed_return(ll::connector::connect_to(
        network.addr, &network.reactor, &network.timermgr, 
        ll::time_prec_msec::to_timeval(2000),
        ll::time_prec_msec::to_timeval(2000), connect_handler));

    while (1) {
        ll::timeval t = network.timermgr.loop();
        ll_failed_return(network.reactor.loop(t));
    }
    return 0;
}


