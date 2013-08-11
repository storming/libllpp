#include <stdio.h>
#include <stdlib.h>
#include "etc.h"

namespace ll {

void terminate() 
{
    abort();
}

void memory_fail()
{
    fprintf(stderr, "memory except.\n");
    terminate();
}

};


