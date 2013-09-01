#include <iostream>
#include <cstdlib>
#include "libll++/timeval.h"
#include "libll++/pool.h"
using std::cout;
using std::endl;


int main()
{
    cout << ll::pool::global() << endl;

    do {
        ll::time_trace t;
        for (unsigned i = 0; i < 1000000; i++) {
            ll::pool::global()->alloc(rand() % 64);
        }
        cout << t.check() << endl;
    } while (0);

    ll::pool *pool = ll::pool::create(ll::pool::global());

    do {
        ll::time_trace t;
        for (unsigned i = 0; i < 1000000; i++) {
            pool->alloc(rand() % 64);
        }
        cout << t.check() << endl;
    } while (0);

    do {
        ll::time_trace t;
        ll::pool::destroy(pool);
        cout << t.check() << endl;
    } while (0);

    char *s = ll::pool::global()->sprintf("hello pool %d\n", 100);
    cout << s;
    return 0;
}
