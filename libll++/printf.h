#ifndef __LIBLLPP_PRINTF_H__
#define __LIBLLPP_PRINTF_H__

#include <cstdarg>

namespace ll {

class printf_formatter {
public:
    struct buff {
        char *curpos;
        char *endpos;
        virtual int flush() = 0;
    };

public:
    static int format(buff *vbuff, const char *fmt, va_list ap);

};

};

#endif

