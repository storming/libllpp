#ifndef __LIBLLPP_LOG_H__
#define __LIBLLPP_LOG_H__

#include <cstdarg>

namespace ll {

struct log_printer {
    virtual void *prepare(int type) = 0;
    virtual void vprint(void *context, const char *fmt, va_list ap) = 0;
    virtual void flush(void *context) = 0;
    virtual void close() = 0;
};

enum {
    log_type_debug,
    log_type_info,
    log_type_error,
};

class log {
private:
    static log_printer *_printer;
    int _type;
    void *_context;
    bool _ready;

public:
    log(int type) : _type(type), _ready(false) {}
    void vprint(const char *fmt, va_list ap) {
        if (!_ready) {
            _context = _printer->prepare(_type);
            _ready = true;
        }
        _printer->vprint(_context, fmt, ap);
    }

    void print(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprint(fmt, ap);
    }

    void flush() {
        if (_ready) {
            _printer->flush(_context);
            _ready = false;
        }
    };

    void vprintf(const char *fmt, va_list ap) {
        vprint(fmt, ap);
        flush();
    }

    void printf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprint(fmt, ap);
        flush();
    }

    void operator()(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprint(fmt, ap);
        flush();
    }

    static void set_printter(log_printer *printer = nullptr);
    static log_printer *get_printer();
};


extern log info;
extern log debug;
extern log error;

}
#endif
