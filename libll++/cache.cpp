#include <new>

#include "cache.h"
#include "module.h"
#include "memory.h"

namespace ll {

caches *caches::_instance = nullptr;

caches::caches () noexcept
{
    cache *c;
    pool *pool = pool::global();

    c = _caches = (cache*)pool->alloc(max_index * sizeof(cache));
    for (unsigned i = 0; i < max_index; i++, c++) {
        new(c) cache((i + 1) << ll_align_order, pool_allocator(pool));
    }
};

struct caches_module {
    int module_init() {
        static caches tmp;
        caches::_instance = &tmp;
        return 0;
    }
};

ll_module(caches_module);

}

