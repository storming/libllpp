#include "memory.h"
#include "module.h"

namespace ll {

mallocator mallocator::_instance;

struct memory_module {
    int module_init() {
        return 0;
    }
};

ll_module(memory_module);

}
