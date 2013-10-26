#include <time.h>
#include <cstdio>
#include "log.h"
#include "memory.h"
#include "timeval.h"

namespace ll {

class default_printer : public log_printer {
public:
    static obstack *_cache;

    void *prepare(int type) {
        obstack *pool;
        if (_cache) {
            pool = _cache;
            _cache = nullptr;
        }
        else {
            pool = ll::_new<obstack>();
        }

        struct ::timeval tv;
        struct ::tm tm;
        ::gettimeofday(&tv, nullptr);
        ::gmtime_r(&tv.tv_sec, &tm);

        pool->print("%02d:%02d:%02d.%02d",
                    tm.tm_hour, tm.tm_min, tm.tm_sec, 
                    (int)time_prec_msec::to_precval(tv.tv_usec));
        switch (type) {
        case log_type_debug:
            pool->grow(":D", 2);
            break;
        case log_type_info:
            pool->grow(":I", 2);
            break;
        case log_type_error:
            pool->grow(":E", 2);
            break;
        }

        pool->grow(": ", 2);
        return pool;
    }

    void vprint(void *ctx, const char *fmt, va_list ap) {
        obstack *pool = (obstack*)ctx;
        pool->vprint(fmt, ap);
    }

    void flush(void *ctx) {
        obstack *pool = (obstack*)ctx;
        pool->grow('\0');
        fputs((char*)pool->finish(), stderr);

        if (!_cache) {
            pool->clear();
            _cache = pool;
        }
        else {
            ll::_delete<obstack>(pool);
        }
    }

    void close() {
        if (_cache) {
            _delete<obstack>(_cache);
            _cache = nullptr;
        }
    }
};

obstack *default_printer::_cache = nullptr;
static default_printer __default_printer;
/* log */
log_printer *log::_printer = &__default_printer;
log info = log(log_type_info);
log debug = log(log_type_debug);
log error = log(log_type_error);

void log::set_printter(log_printer *printer) 
{
    if (!printer) {
        printer = &__default_printer;
    }
    if (log::_printer == printer) {
        return;
    }
    log::_printer->close();
    log::_printer = printer;
}

log_printer *log::get_printer() 
{
    return log::_printer;
}


}
