#ifndef __LIBLLPP_ERROR_H__
#define __LIBLLPP_ERROR_H__

#include <errno.h>

namespace ll {

#define ll_sys_rc(x) (-10000 - (x))

enum rc {
    success,
    ok = success,
    exists,
    notexists,
    timedout,
    notready,
    busy,
    full,
    empty,

    fail = -1,
    e_sys_begin = -10000,
    e_closed,
    e_exists,
    e_notexists,
    e_ready,
    e_notready,

    e_busy = ll_sys_rc(EBUSY),
    e_timedout = ll_sys_rc(ETIMEDOUT),
    e_inval = ll_sys_rc(EINVAL),
};

}


#endif

