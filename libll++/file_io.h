#ifndef __LIBLLPP_FILE_IO_H__
#define __LIBLLPP_FILE_IO_H__

#include "etc.h"

#define ll_fd_valid(fd) ((fd) >= 0)

namespace ll {

class file_io {
private:
    int _fd;

public:
    file_io() : _fd(-1) {}
    file_io(int fd) : _fd(fd) {}
    ~file_io() {
        if (ll_fd_valid(_fd)) {
            close();
        }
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

    int close() {
        ll_failed_return(close(_fd));
        _fd = -1;
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
