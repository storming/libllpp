#ifndef __LIBLLPP_FILE_IO_H__
#define __LIBLLPP_FILE_IO_H__

#include <utility>
#include "etc.h"

#define ll_fd_valid(fd) ((fd) >= 0)

namespace ll {

class file_io {
private:
    int _fd;

    void swap(file_io &x) {
        std::swap(_fd, x._fd);
    }
public:
    file_io() noexcept : _fd(-1) {}
    file_io(int fd) noexcept : _fd(fd) {}
    file_io(const file_io &x) : _fd(x._fd) {}
    file_io(file_io &&x) {
        swap(x);
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
        swap(x);
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

    void attach(int fd) {
        _fd = fd;
    }

    void deattch() {
        _fd = -1;
    }

    static int close(int fd);
    int read(void *buf, size_t size);
    int write(const void *buf, size_t size);
    int set_block(bool block);
    int get_block();
};

}

#endif
