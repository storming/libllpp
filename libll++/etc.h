#ifndef __LIBLLPP_ETC_H__
#define __LIBLLPP_ETC_H__

#include <cstddef>

#define ll_is_p2aligned(x, a)   ((((uintptr_t)(v)) & ((uintptr_t)(a) - 1)) == 0)
#define ll_is_p2(x)             (((x) & ((x) - 1)) == 0)
#define ll_p2align(x, a)        ((x) & -(align))
#define ll_align(x, a)          (((x) + ((a) - 1)) & ~((a) - 1))
#define ll_align_order          3
#define ll_align_size           (1 << ll_align_order)
#define ll_align_default(x)     ll_align(x, ll_align_size)

#define ll_likely(x)            __builtin_expect(!!(x), 1)
#define ll_unlikely(x)          __builtin_expect(!!(x), 0)

namespace ll {

void terminate();
void memory_fail();

};


#endif

