#include "runtime/state.h"

#include <stdio.h>

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Initialization.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>

#include "runtime/modules.h"


tulip_runtime_state tulip_runtime_start() {
  tulip_runtime_state state;

  printf("initializing llvm\n");

  LLVMPassRegistryRef pass = LLVMGetGlobalPassRegistry();

  LLVMInitializeCore(pass);

  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllTargets();
  LLVMInitializeAllTargetMCs();

  state.modules = malloc(sizeof(tulip_runtime_module));
  state.num_modules = 1;
  state.modules[0] = main_module;
  state.modules[0].llvm_module = LLVMModuleCreateWithName(state.modules[0].name);

  // mcjit symbols are not linked by default
  LLVMLinkInMCJIT();
  struct LLVMMCJITCompilerOptions o;
  LLVMInitializeMCJITCompilerOptions(&o, sizeof(o));
  // jit option flags can be set here
  // we'll want a little more control over host symbol linking in the future, and the best way to do that is to override the MemoryManager here
  char* error = malloc(sizeof(char) * 32);

  // it's likely that we'll want multiple modules (one for each process) with a shared mcjit instance, but for now it's safe to run it like this
  if(LLVMCreateMCJITCompilerForModule(&(state.jit_instance), state.modules[0].llvm_module, &o, sizeof(o), &error)) {
    // [wtf] LLVMBool true means an error occured
    printf("error occured:\n\t");
    printf("%s\n", error);
  } else {
    printf("initialized mcjit\n");
  }

  return state;
}

void tulip_runtime_load_module(tulip_runtime_state* state, tulip_runtime_module mod) {
  state->modules = realloc(state->modules, sizeof(tulip_runtime_module) * state->num_modules + 1);
  state->modules[state->num_modules] = mod;
  state->num_modules += 1;
}

void tulip_runtime_stop(tulip_runtime_state state) {
  // [todo] destroy llvm modules
  LLVMDisposeExecutionEngine(state.jit_instance);
  LLVMShutdown();
}
