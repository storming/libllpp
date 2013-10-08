#include <new>

#include "cache.h"
#include "module.h"
#include "memory.h"

namespace ll {

caches *caches::_instance = nullptr;

struct caches_module {
    int module_init() {
        static caches tmp;
        caches::_instance = &tmp;
        return 0;
    }
};

ll_module(caches_module);

}

