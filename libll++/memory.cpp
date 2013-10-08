#include "memory.h"
#include "module.h"

namespace ll {

struct memory_module {
    int module_init() {
        return 0;
    }
};

ll_module(memory_module);

}
