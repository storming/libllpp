#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "etc.h"

namespace ll {

void terminate() 
{
    abort();
}

void crit_error(const char *msg, int errnum) 
{
    if (errnum) {
        fprintf(stderr, "%s: %s.\n", msg, strerror(errnum));
    }
    else {
        fprintf(stderr, "%s\n", msg);
    }
    terminate();
}

void memory_fail()
{
    crit_error("out of memory.");
}

};


