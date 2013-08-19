#include "module.h"
#include <stdio.h>
#include <stdlib.h>

namespace ll {
namespace internal {
namespace module {

bool __modules_reverse = false;
static modules_runner __attribute__((used)) __runner(false);

unsigned modules_runner::_counter = 0;
modules_runner::modules_runner(bool end)
{
    if (_counter) {
        if (!end) {
            __modules_reverse = true;
        }
        modules::instance().init();
    }
    _counter++;
}

modules_runner::~modules_runner() 
{
    _counter--;
    if (!_counter) {
        modules::instance().exit();
    }
}

};
};
};
