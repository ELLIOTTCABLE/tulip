#include "runtime/modules.h"

// the main module is intended to contain the entry point, and can be specialized for repl execution
// it is only temporarily a constant
const tulip_runtime_module main_module = (tulip_runtime_module){ "main_module", {0, 0, 0}, TULIP_MODULE_EMPTY, NULL, 0, NULL};

const tulip_runtime_module_version null_module_version = {0, 0, 0};
