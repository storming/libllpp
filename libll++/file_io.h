#ifndef __LIBLLPP_FILE_IO_H__
#define __LIBLLPP_FILE_IO_H__

#include <utility>
#include "etc.h"

#define ll_fd_valid(fd) ((fd) >= 0)

namespace ll {

class file_io {
private:
    int _fd;

public:
    file_io() noexcept : _fd(-1) {}
    file_io(int fd) noexcept : _fd(fd) {}
    file_io(const file_io &x) noexcept : _fd(x._fd) {}
    file_io(file_io &&x) noexcept : _fd(-1) {
        std::swap(_fd, x._fd);
    }
    ~file_io() {
        if (ll_fd_valid(_fd)) {
            file_io::close(_fd);
        }
    }
    
    file_io &operator=(const file_io &x) noexcept {
        _fd = x._fd;
        return *this;
    }

    file_io &operator=(file_io &&x) noexcept {
        std::swap(_fd, x._fd);
        return *this;
    }

    file_io &operator=(int fd) noexcept {
        _fd = fd;
        return *this;
    }

    operator int() {
        return _fd;
    }

    int open(int fd) {
        if (!ll_fd_valid(fd)) {
            return e_inval;
        }
        if (ll_fd_valid(_fd)) {
            return e_busy;
        }
        _fd = fd;
        return fd;
    }
    
    bool opened() {
        return ll_fd_valid(_fd);
    }

    virtual int close() {
        if (ll_fd_valid(_fd)) {
            ll_failed_return(file_io::close(_fd));
            _fd = -1;
        }
        return ok;
    }

    int attach(int fd) {
        int tmp = _fd;
        _fd = fd;
        return tmp;
    }

    int deattch() {
        int tmp = _fd;
        _fd = -1;
        return tmp;
    }

    static int close(int fd);
    int read(void *buf, size_t size);
    int write(const void *buf, size_t size);
    int set_block(bool block);
    int get_block();
};

}

#endif
