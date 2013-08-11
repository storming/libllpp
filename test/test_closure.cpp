#include <iostream>
#include <functional>
using std::cout;
using std::endl;

#include "libll++/closure.h"

struct foo {
    foo() {
    }
    void operator ()(int &n, int m) {
        cout << n++ + m << endl;
    }

    void dodo(int &n, int m) {
        cout << "dodo:" << n++ + m << endl;
    }
};

int main()
{
    int n = 1;
    foo f;
    ll::closure<foo, int&> c1(f, n);
    c1(100);
    c1(100);

    auto c2 = ll::the_closure(f, n);
    c2(100);
    c2(100);

    ll::closure<decltype(&foo::dodo), int&> c3(f, &foo::dodo, n);
    c3(100);
    c3(100);

    auto c4 = ll::the_closure(f, &foo::dodo, n);
    c4(100);
    c4(100);

    return 0;
}
