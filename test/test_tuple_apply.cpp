#include <iostream>
#include "libll++/tuple_apply.h"
#include "libll++/timeval.h"

struct A {
    int operator()(int n1, int n2, int n3, int n4) {
        std::cout << n1 << " "
                  << n2 << " "
                  << n3 << " "
                  << n4 << std::endl;
        return n1 + n2 + n3 + n4;
    }
};

struct B {
    volatile int _sum;
    B(): _sum() {}
    void operator()(int n1, int n2, int n3, int n4) {
        _sum = n1 + n2 + n3 + n4;
    }
};

#define COUNT 100000000
int main()
{
    A a;
    std::tuple<int, int> t(3, 4);
    ll::tuple_apply(a, t, 1, 2);
    ll::tuple_apply<false>(a, t, 1, 2);

    B b;
    unsigned count;
    ll::time_trace tt;

    count = COUNT;
    while (count--) {
        ll::tuple_apply(b, t, 1, 2);
    }
    std::cout << tt.check() << std::endl;

    tt.reset();

    count = COUNT;
    while (count--) {
        b(1, 2, 3, 4);
    }
    std::cout << tt.check() << std::endl;


    return 0;
}

