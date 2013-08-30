#include <type_traits>
#include <iostream>
using std::cout;
using std::endl;

#include "libll++/slotsig.h"
#include "libll++/time.h"
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
    int dodo() {
        cout << "kkkkkkkkkk" << endl;
        return 0;
    }
    ll::signal<int()>::member<decltype(&B::dodo)> _slot;

    B() : _slot(this, &B::dodo) {}
};

int main()
{
    foo f;
    B b;
    ll_member_slot(f.sig, B, dodo) ss(b, &B::dodo);
    f.sig.connect(b._slot);
    //f.sig.connect(ll::pool::global(), b, &B::dodo);
    f.sig.emit();
    //cout << &ll::internal::module::__modules << endl;
    return 0;
}
