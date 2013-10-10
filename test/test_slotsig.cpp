#include <iostream>
#include <type_traits>
#include <utility>

using std::cout;
using std::endl;

#include "libll++/slotsig.h"
#include "libll++/memory.h"

struct foo {

    int _n;
    const char *_p;
    int operator()(int &magic) {
        cout << "bbbbbbbbbbb" << magic++ << endl;
        return 0;
    }

    int dodo(int magic) {
        cout << "kkkkkkkkkk" << magic++ << endl;
        return 0;
    }
    
    foo() {}
    foo(const char *p) : _p(p) {}
    foo(int n, const char *p) : _n(n), _p(p) {}

};

int main()
{
    foo f;
    int n = 100;

    ll::signal<int(), ll::malloc_allocator> sig;
    sig.connect(f, n);
    auto slot = sig.connect(&foo::dodo, f, 100);
    sig.emit();
    sig.disconnect(slot);
    sig.emit();

    ll::signal<int(), ll::mallocator, true> once_sig;
    once_sig.connect(f, n);
    once_sig.emit();
    once_sig.connect(&foo::dodo, f, 100);
    once_sig.emit();
    return 0;
}
