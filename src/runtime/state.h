#pragma once

#include <stdlib.h>
#include <stdbool.h>

// todo move these macro definitions somewhere more sensible
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>

#include "runtime/regions.h"
#include "runtime/processes.h"

typedef enum {
  runtime_initialized,
  runtime_busy,
  runtime_paused
} tulip_runtime_status;

typedef struct tulip_runtime_options {
  unsigned int heap_size;
  unsigned int module_limit;
} tulip_runtime_options;

typedef struct tulip_runtime_state {
  value_region*          values;
  module_region*         modules;
  module_ref             main_module;
  tulip_runtime_process  proc; // [note] there is currently only one process at a time
  tulip_runtime_status   status;
  LLVMExecutionEngineRef jit_instance;

} tulip_runtime_state;

tulip_runtime_state tulip_runtime_start();
void tulip_runtime_stop(tulip_runtime_state rt);
