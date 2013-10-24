#include <iostream>
#include <functional>

using std::cout;
using std::endl;

#include "libll++/functor.h"
#include "libll++/guard.h"

template <typename _T>
void test(_T &&obj) {
    auto fn = ll::make_fn(std::forward<_T>(obj));
    fn();
};

struct foo {

    void operator()() {
        cout << "operator" << endl;
    }

    void operator()() const {
        cout << "operator const" << endl;
    }

    void test() {
        ::test(this);
    }

    void test() const {
        ::test(this);
    }

    void test2() const {
        ::test(this);
    }
};

void test1() {
    cout << "test1" << endl;
}

void test2(int n) {
    cout << "test2" << endl;
}

void test3(int n)
{
    if (!n) {
        return;
    }

    auto guard = ll::make_guard([](){
        cout << "test3 lambda guard" << endl;
    });

    return;
}
int main()
{
    do {
        auto guard = ll::make_guard([](){
            cout << "lambda guard" << endl;
        });
    } while (0);

    do {
        auto guard = ll::make_guard(test1);
    } while (0);

    do {
        foo f;
        auto guard = ll::make_guard(&foo::test2, f);
    } while (0);

    do {
        foo f;
        auto fn = ll::make_fn(f);
        fn();
    } while (0);

    do {
        foo f;
        foo &ref = f;
        auto fn = ll::make_fn(ref);
        fn();
    } while (0);

    do {
        auto fn = ll::make_fn(new foo);
        fn();
    } while (0);

    do {
        const foo f;
        auto fn = ll::make_fn(f);
        fn();
    } while (0);

    do {
        foo f;
        const foo &ref = f;
        auto fn = ll::make_fn(ref);
        fn();
    } while (0);

    do {
        auto fn = ll::make_fn((const foo*)new foo);
        fn();
    } while (0);

    do {
        foo f;
        auto fn = ll::make_fn(f);
        fn();
    } while (0);

    do {
        foo f;
        auto fn = ll::make_fn(&foo::test2);
        fn(f);
    } while (0);

    do {
        auto fn = ll::make_fn([](){
            cout << "lambda" << endl;
        });
        fn();
    } while (0);

    do {
        auto fn = ll::make_fn(test1);
        fn();
    } while (0);

    do {
        auto p = &test1;
        auto fn = ll::make_fn(p);
        fn();
    } while (0);

    do {
        auto fn = ll::make_functor<void()>(test2, 1);
        fn();
    } while (0);

    do {
        foo f;
        auto fn = ll::make_functor<void()>(f);
        fn();
    } while (0);

    do {
        const foo f;
        auto fn = ll::make_functor<void()>(f);
        fn();
    } while (0);

    do {
        foo f;
        auto fn = ll::make_functor<void()>(&foo::test2, f);
        fn();
    } while (0);

    do {
        auto fn = ll::make_functor<void()>([](){
            cout << "lambda functor" << endl;
        });
        fn();
    } while (0);
    return 0;
}
