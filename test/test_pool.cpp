#include <iostream>
#include <cstdlib>
#include "libll++/timeval.h"
#include "libll++/pool.h"
#include "libll++/guard.h"
#include "libll++/etc.h"

using std::cout;
using std::endl;

void bye() {
    cout << "bye" << endl;
}

void test() {
    //auto a = ll::make_guard();
    cout << "bbbbbbbbbbbb" << endl;
}

#define COUNT 100000000
int main()
{
    test();
    return 0;
    do {
        ll::time_trace t;
        for (unsigned i = 0; i < 1000000; i++) {
            ll::pool::global()->alloc(rand() % 64);
        }
        cout << t.check() << endl;
    } while (0);

    ll::pool *pool = ll::_new<ll::pool>(ll::pool::global());
    pool->connect(bye);

    do {
        ll::time_trace t;
        for (unsigned i = 0; i < 1000000; i++) {
            pool->alloc(rand() % 64);
        }
        cout << t.check() << endl;
    } while (0);

    do {
        ll::time_trace t;
        ll::_delete<ll::pool>(pool);
        cout << t.check() << endl;
    } while (0);

    char *s = ll::pool::global()->sprintf("hello pool %d\n", 100);
    cout << s;

    return 0;
}
