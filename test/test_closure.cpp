#include <iostream>
#include <functional>
using std::cout;
using std::endl;

#include "libll++/memory.h"
#include "libll++/closure.h"

struct foo {
    foo() {
    }
    void operator ()(int &n, int m) {
        cout << n++ + m << endl;
    }

    void dodo(int n, int m) const {
        cout << "dodo:" << n++ + m << endl;
    }

    static void test() {
    }
};

struct foo2 : foo {
};

void func(int n, int m) {
    cout << "ffffffffffff " << m << endl;
}

typedef void (*func_t)(int, int);

int main()
{
    int n = 1;
    foo2 f;

    foo *p = &f;

    do {
        auto c = ll::closure<void(int)>::make(&foo2::dodo, p, n);
        c(100);
    } while (0);

    do {
        auto c = ll::closure<void(int)>::make(func, n);
        c(100);
    } while (0);

    do {
        auto c = ll::closure<void(int)>::make(&func, n);
        c(100);
    } while (0);

    do {
        func_t fn = func;
        auto c = ll::closure<void(int)>::make(fn, n);
        c(100);
    } while (0);

    do {
        auto c = ll::closure<void(int)>::make(f, n);
        c(100);
    } while (0);

    do {
        auto c = ll::closure<void(int)>::make(&f, n);
        c(100);
    } while (0);

    do {
        foo *p = &f;
        auto c = ll::closure<void(int)>::make(p, n);
        c(100);
    } while (0);

    do {
        auto c = ll::closure<void(int)>::make(foo(), n);
        c(100);
    } while (0);

    do {
        auto c = ll::closure<void(int)>::make([&](int m){
            cout << "lambda:" << n++ + m << endl;
        });
        c(100);
    } while (0);

    do {
        auto fn = [&](int m){
            cout << "lambda:" << n++ + m << endl;
        };

        auto c = ll::closure<void(int)>::make(fn);
        c(100);
    } while (0);

    do {
        auto fn = [&](int m){
            cout << "lambda:" << n++ + m << endl;
        };

        auto c = ll::closure<void(int)>::make(&fn);
        c(100);
    } while (0);

    do {
        auto fn = [&](int m){
            cout << "lambda:" << n++ + m << endl;
        };

        auto c = ll::closure<void(int)>::make(std::move(fn));
        c(100);
    } while (0);

    ll::closure<void(int)> *c = ll::closure<void(int)>::_new(ll::malloc_allocator(), f, n);
    return 0;
}


