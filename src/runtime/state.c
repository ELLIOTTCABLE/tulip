#include "runtime/state.h"

#include <stdio.h>

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Initialization.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>

tulip_runtime_state tulip_runtime_start(tulip_runtime_options o) {
  tulip_runtime_state state;

  state.modules = init_module_region(o.module_limit);
  module_ref main = region_insert_module(state.modules, main_module);

  // [beta] there is only one heap region, and it is attached to the runtime state
  //        in the future, there may be individual regions per process, but the collection strategy hasn't been decided yet
  state.values = init_value_region(o.heap_size);

  ////////////////////////////////////////////////////////////////////////////////
  // global llvm setup

  LLVMPassRegistryRef pass = LLVMGetGlobalPassRegistry();

  LLVMInitializeCore(pass);
  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllTargets();
  LLVMInitializeAllTargetMCs();

  LLVMLinkInMCJIT();
  struct LLVMMCJITCompilerOptions opts;
  LLVMInitializeMCJITCompilerOptions(&opts, sizeof(opts));

  // [todo] execution engine instantiation

  return state;
}

// tulip_runtime_call(module_ref module, const char* function)

void tulip_runtime_stop(tulip_runtime_state state) {
  // [todo] free modules in module region
  // [todo] free all values
  LLVMDisposeExecutionEngine(state.jit_instance);
  LLVMShutdown();
}
