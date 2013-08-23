#include <new>

#include "cache.h"
#include "module.h"
#include "memory.h"

namespace ll {

caches *caches::_global = nullptr;

caches::caches(pool *p)
{
    assert(p);

    cache *c;
    c = _caches = (cache*)p->alloc(max_index * sizeof(cache));
    for (unsigned i = 0; i < max_index; i++, c++) {
        new(c) cache((i + 1) << ll_align_order, p);
    }
}

struct cache_module {
    int module_init() {
        caches::_global = create<caches>(pool::global(), pool::global());
        return 0;
    }
};

ll_module(cache_module);

}

