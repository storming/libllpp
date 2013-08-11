#include <iostream>
#include <cstddef>
#include "libll++/page.h"
#include "libll++/etc.h"
#include "libll++/time.h"

#include <sys/mman.h>
#include <unistd.h>

using std::cout;
using std::endl;


int main()
{
    ll::page_allocator a;

    do {
        ll::time_trace t;
        for (unsigned i = 0; i < 1024 * 1024; i++) {
            ll::page *pg1 = a.alloc(1);
            ll::page *pg2 = a.alloc(1);
            a.free(pg2);
            a.free(pg1);
        }
        cout << t.check() / 1000 << endl;
    } while (0);

    do {
        ll::time_trace t;
        for (unsigned i = 0; i < 1024 * 1024; i++) {
            void *p = malloc(8192);
            void *p2 = malloc(8192);
            free(p2);
            free(p);
        }
        cout << t.check() / 1000 << endl;
    } while (0);

    return 0;
}
