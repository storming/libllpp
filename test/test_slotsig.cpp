#include <iostream>
using std::cout;
using std::endl;

#include "libll++/slotsig.h"
#include "libll++/memory.h"

struct foo {
    int operator()(int &magic) {
        cout << "bbbbbbbbbbb" << magic++ << endl;
        return 0;
    }

    int dodo(int magic) {
        cout << "kkkkkkkkkk" << magic++ << endl;
        return 0;
    }
    
    int _n;

    void free(void *, size_t) {
    }
};

struct fofo {
    void free(void *, size_t) {
    }
};

struct foo2 : foo, fofo {
    using fofo::free;
    void lll() {
    }
};

template <typename _T>
struct member_class;

template <typename _T, typename _U>
struct member_class<_T _U::*> {
    typedef _U type;
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
