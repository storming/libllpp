#ifndef __LIBLLPP_ETC_H__
#define __LIBLLPP_ETC_H__

#include <cstddef>
#include "rc.h"

#define ll_is_p2aligned(x, a)   ((((uintptr_t)(v)) & ((uintptr_t)(a) - 1)) == 0)
#define ll_is_p2(x)             (((x) & ((x) - 1)) == 0)
#define ll_p2align(x, a)        ((x) & -(align))
#define ll_align(x, a)          (((x) + ((a) - 1)) & ~((a) - 1))
#define ll_align_order          3
#define ll_align_size           (1 << ll_align_order)
#define ll_align_default(x)     ll_align(x, ll_align_size)

#define ll_likely(x)            __builtin_expect(!!(x), 1)
#define ll_unlikely(x)          __builtin_expect(!!(x), 0)

#define ll_successed(x)         ((x) >= 0)
#define ll_ok(x)                ((x) >= 0)
#define ll_failed(x)            ((x) < 0)
#define ll_sys_failed(x)        ((x) == -1)

#define ll_failed_return(x)                   \
    do {                                      \
        int __ret = (x);                      \
        if (ll_unlikely(ll_failed(__ret))) {  \
            return __ret;                     \
        }                                     \
    } while (0)

#define ll_failed_return_ex(x, cmd)           \
    do {                                      \
        int __ret = (x);                      \
        if (ll_unlikely(ll_failed(__ret))) {  \
            {cmd;}                            \
            return __ret;                     \
        }                                     \
    } while (0)

#define ll_sys_failed_return(x)               \
    do {                                      \
        if (ll_unlikely(ll_sys_failed(x))) {  \
            return ll_sys_rc(errno);          \
        }                                     \
    } while (0)

#define ll_sys_failed_return_ex(x, cmd)       \
    do {                                      \
        if (ll_unlikely(ll_sys_failed(x))) {  \
            int __errnum = ll_sys_rc(errno);  \
            {cmd;}                            \
            return __errnum;                  \
        }                                     \
    } while (0)

#define ll_successed_return(x)                \
    do {                                      \
        int __ret = (x);                      \
        if (ll_likely(ll_successed(__ret))) { \
            return __ret;                     \
        }                                     \
    } while (0)

#define ll_ok_return(x)                       \
    do {                                      \
        int __ret = (x);                      \
        if (ll_likely(ll_successed(__ret))) { \
            return __ret;                     \
        }                                     \
    } while (0)

namespace ll {

void terminate();
void crit_error(const char *msg, int errnum = 0);
void memory_fail();

};


#endif

