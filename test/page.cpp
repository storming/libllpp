#include <iostream>
#include <cstddef>
#include "libll++/page.h"
#include "libll++/etc.h"
#include "libll++/time.h"

#include <sys/mman.h>
#include <unistd.h>

using std::cout;
using std::endl;

void test(int PTS) 
{
    void *pts[PTS];

#define COUNT 100000000
    cout << "PTS: " << PTS << endl;

    do {
        memset(pts, 0, sizeof(pts));
        ll::time_trace t;
        for (unsigned i = 0; i < COUNT; i++) {
            unsigned n = i % PTS;
            if (pts[n]) {
                ll::page_allocator::global()->free((ll::page*)pts[n]);
            }
            pts[n] = ll::page_allocator::global()->alloc(1);
        }
        cout << t.check() / 1000 << endl;
    } while (0);

    do {
        memset(pts, 0, sizeof(pts));
        ll::time_trace t;
        for (unsigned i = 0; i < COUNT; i++) {
            unsigned n = i % PTS;
            if (pts[n]) {
                free(pts[n]);
            }
            pts[n] = malloc(8192);
        }
        cout << t.check() / 1000 << endl;
    } while (0);
}

int main()
{
    for (int i = 1; i < 16; i++) {
        test(i);
    }
    return 0;
}
