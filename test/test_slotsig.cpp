#include <iostream>
using std::cout;
using std::endl;

#include "libll++/slotsig.h"
#include "libll++/time.h"


struct foo {
    ll::signal<void()> sig;
};

struct B {
    void dodo() {
        cout << "kkkkkkkkkk" << endl;
    }
    ll::signal<void()>::member<decltype(&B::dodo)> _slot;

    B() : _slot(this, &B::dodo) {}
};

int main()
{
    foo f;
    B b;
    f.sig.connect(b._slot);
    f.sig.emit();
    return 0;
}
