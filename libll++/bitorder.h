#ifndef __LIBLLPP_BITORDER_H__
#define __LIBLLPP_BITORDER_H__

namespace ll {
struct bitorder {
    static const unsigned char ordertab[];

    /* (1 << order(n)) >= n */
    static unsigned order(unsigned long n) {
        register unsigned r = 0;
        register unsigned long m = n;
        while (m & (~0xff)) {
            m >>= 8;
            r += 8;
        }
        r += ordertab[m];
        if ((1UL << r) < n) {
            r++;
        }
        return r;
    }


};
};

#endif

