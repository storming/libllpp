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
        auto c = ll::make_closure<void(int)>(&foo2::dodo, p, n);
        c(100);
    } while (0);

    do {
        auto c = ll::make_closure<void(int)>(func, n);
        c(100);
    } while (0);

    do {
        auto c = ll::make_closure<void(int)>(&func, n);
        c(100);
    } while (0);

    do {
        func_t fn = func;
        auto c = ll::make_closure<void(int)>(fn, n);
        c(100);
    } while (0);

    do {
        auto c = ll::make_closure<void(int)>(f, n);
        c(100);
    } while (0);

    do {
        auto c = ll::make_closure<void(int)>(&f, n);
        c(100);
    } while (0);

    do {
        foo *p = &f;
        auto c = ll::make_closure<void(int)>(p, n);
        c(100);
    } while (0);

    do {
        auto c = ll::make_closure<void(int)>(foo(), n);
        c(100);
    } while (0);

    do {
        auto c = ll::make_closure<void(int)>([&](int m){
            cout << "lambda:" << n++ + m << endl;
        });
        c(100);
    } while (0);

    do {
        auto fn = [&](int m){
            cout << "lambda:" << n++ + m << endl;
        };

        auto c = ll::make_closure<void(int)>(fn);
        c(100);
    } while (0);

    do {
        auto fn = [&](int m){
            cout << "lambda:" << n++ + m << endl;
        };

        auto c = ll::make_closure<void(int)>(&fn);
        c(100);
    } while (0);

    do {
        auto fn = [&](int m){
            cout << "lambda:" << n++ + m << endl;
        };

        auto c = ll::make_closure<void(int)>(std::move(fn));
        c(100);
    } while (0);

    return 0;
}


