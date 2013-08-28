#ifndef __LIBLLPP_COMPARE_H__
#define __LIBLLPP_COMPARE_H__

namespace ll {


template <typename _T>
struct comparer {
    static int compare(_T obj, _T other) {
        if (obj > other) {
            return 1;
        }
        else if (obj < other) {
            return -1;
        }
        else {
            return 0;
        }
    }
};

template <typename _T>
struct equal_compare {
    static int compare(_T obj, _T other) {
        return obj != other;
    }
};

}
#endif
