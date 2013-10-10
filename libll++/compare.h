#ifndef __LIBLLPP_COMPARE_H__
#define __LIBLLPP_COMPARE_H__

namespace ll {


template <typename _T>
struct comparer {
    static int compare(_T x, _T y) noexcept {
        if (x > y) {
            return 1;
        }
        else if (x < y) {
            return -1;
        }
        else {
            return 0;
        }
    }
};

template <typename _T>
struct equal_compare {
    static int compare(_T x, _T y) noexcept {
        return x != y;
    }
};

}
#endif
