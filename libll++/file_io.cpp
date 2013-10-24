#include <cassert>
#include <unistd.h>
#include <fcntl.h>

#include "file_io.h"

namespace ll {

int file_io::close(int fd) {
    while (1) {
        if (ll_likely(ll_ok(::close(fd)))) {
            return ok;
        }

        if (errno == EINTR) {
            continue;
        }

        return ll_sys_rc(errno);
    }
}

int file_io::set_block(bool block) 
{
    int n;
    ll_sys_failed_return(n = ::fcntl(_fd, F_GETFL));

    if (block) {
        n &= ~O_NONBLOCK;
    }
    else {
        n |= O_NONBLOCK;
    }

    ll_sys_failed_return(::fcntl(_fd, F_SETFL, n));
    return ok;
}

int file_io::get_block() 
{
    int n;
    ll_sys_failed_return(n = ::fcntl(_fd, F_GETFL));

    return !(n & O_NONBLOCK);
}

int file_io::read(void *buf, size_t size) 
{
    char *p = (char*)buf;
    int n;
    while (size) {
        n = ::read(_fd, p, size);
        if (ll_likely(n > 0)) {
            size -= n;
            p += n;
        }
        else if (ll_likely(n < 0)) {
            if (ll_likely(errno == EAGAIN)) {
                break;
            }
            else if (ll_likely(errno == EINTR)) {
                continue;
            }
            else {
                return ll_sys_rc(errno);
            }
        }
        else {
            return e_closed;
        }
    }
    return p - (char*)buf;
}

int file_io::write(const void *buf, size_t size)
{
    const char *p = (const char*)buf;

    int n;
    while (size) {
        n = ::write(_fd, p, size);
        if (ll_likely(n > 0)) {
            size -= n;
            p += n;
        }
        else if (ll_likely(n < 0)) {
            if (ll_likely(errno == EAGAIN)) {
                break;
            }
            else if (ll_likely(errno == EINTR)) {
                continue;
            }
            else {
                return ll_sys_rc(errno);
            }
        }
        else {
            return e_closed;
        }
    }

    return p - (char*)buf;
}


}
