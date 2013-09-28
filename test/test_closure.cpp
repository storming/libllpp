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

    virtual void dodo2() {
        cout << "foo::dodo2" << endl;
    }
};

int main()
{
    int n = 1;
    foo f;

    ll::closure<void(int)> *c;

    ll::closure<void(int)>::instance<foo, int&> c5(f, n);
    c5(100);c5(100);

    c = &c5;
    (*c)(100);(*c)(100);


    ll::closure<void(int)>::instance<foo, int&> c6(f, &foo::dodo, n);
    
    c = &c6;
    (*c)(100);(*c)(100);
    
    int m = 200;
    c->apply(m);

    return 0;
}


