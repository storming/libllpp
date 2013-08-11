#ifndef __LIBLLPP_BITORDER_H__
#define __LIBLLPP_BITORDER_H__

namespace ll {
struct bitorder {
    static const unsigned char ordertab[];

    static unsigned order(unsigned n) {
        unsigned r = 0;
        while (n & (~0xff)) {
            n >>= 8;
            r += 8;
        }
        return r + ordertab[n];
    }

};
};

#endif

