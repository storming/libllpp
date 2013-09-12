#include <type_traits>
#include <iostream>
using std::cout;
using std::endl;

#include "libll++/slotsig.h"
#include "libll++/timeval.h"
#include "libll++/module.h"

struct static_foo {
    static_foo() {
        cout << "static_foo111" << endl;
    }
    static_foo(bool b) {
        cout << "static_foo" << endl;
        cout << b << endl;
    }
};

static static_foo fff;
static static_foo fff2(false);

struct foo {
    foo() {
        cout << "foo" << endl;
    }
    ll::signal<int()> sig;
};

struct B {
    int dodo(int magic) {
        cout << "kkkkkkkkkk" << magic << endl;
        return 0;
    }
    ll::signal<int()>::slot<B, int> _slot;

    B() : _slot(this, &B::dodo, 3) {}
};

void walk(void sum1(int)) 
{
    sum1(0);
}

int main()
{
    foo f;
    B b;
    f.sig.connect(b._slot);
    f.sig.emit();

    return 0;
}
