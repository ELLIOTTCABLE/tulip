#include "runtime/state.h"

#include <stdio.h>
#include <unistd.h>

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Initialization.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>

#include "runtime/transforms.h"

tulip_runtime_state* tulip_runtime_start(tulip_runtime_options o) {
  tulip_runtime_state* state = malloc(sizeof(tulip_runtime_state));

  state->modules = init_module_region(o.module_limit);

  // [beta] there is only one heap region, and it is attached to the runtime state
  //        in the future, there may be individual regions per process, but the collection strategy hasn't been decided yet
  state->values = init_value_region(o.heap_size);

  ////////////////////////////////////////////////////////////////////////////////
  // global llvm setup

  LLVMPassRegistryRef pass = LLVMGetGlobalPassRegistry();

  LLVMInitializeCore(pass);

  // [note] i'd prefer not to initialized all targets in the future
  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllTargets();
  LLVMInitializeAllTargetMCs();

  LLVMLinkInMCJIT();
  struct LLVMMCJITCompilerOptions opts;
  LLVMInitializeMCJITCompilerOptions(&opts, sizeof(opts));

  // [note] these members vary depending on execution mode (file/repl) and are set to null until ee instantiation
  state->main_module = 0;
  state->proc = NULL;
  state->status = runtime_uninitialized;

  LLVMModuleRef null_module = LLVMModuleCreateWithName("null");

  char* ee_err;

  if (LLVMCreateExecutionEngineForModule(&state->jit_instance, null_module, &ee_err)) {
    state->jit_instance = NULL;
    state->status = runtime_failed;

    fprintf(stderr, "%s\n", ee_err);
    free(ee_err);

    return state;
  } else {
    free(ee_err);

    return state;
  }
}

// [pretty] do all of these files need to be repeated?
void tulip_runtime_load_main(tulip_runtime_state* state, main_file_output o) {

  for (unsigned int i = 0; i < o.num_dependencies; i++) {
    tulip_runtime_module dep_mod;

    dep_mod.name = o.dependencies[i].name;
    dep_mod.path_len = o.dependencies[i].path_len;
    // [todo] should be a malloc+memcpy?
    dep_mod.path = o.dependencies[i].path;
    // [todo] module versions
    dep_mod.version = null_module_version;
    dep_mod.status = TULIP_MODULE_UNCOMPILED;
    dep_mod.num_definitions = o.dependencies[i].num_definitions;
    dep_mod.definitions = malloc(sizeof(tulip_runtime_toplevel_definition) * dep_mod.num_definitions);

    for (unsigned int j = 0; j < dep_mod.num_definitions; j++) {
      dep_mod.definitions[j].name = o.dependencies[i].names[j];
      dep_mod.definitions[j].definition = o.dependencies[i].definitions[j];
    }

    // [note] these fields are instantiated during module transform
    dep_mod.llvm_module = NULL;
    dep_mod.module_scope = NULL;
    dep_mod.native_defs = NULL;

    module_ref dep_mod_loaded = region_insert_module(state->modules, dep_mod);

    tulip_runtime_compile_module(state, region_query_module_by_id(state->modules, dep_mod_loaded));

    LLVMAddModule(state->jit_instance, region_query_module_by_id(state->modules, dep_mod_loaded)->llvm_module);
  }

  tulip_runtime_module main_mod;

  main_mod.name = o.main_module.name;
  main_mod.path_len = o.main_module.path_len;
  main_mod.path = o.main_module.path;
  main_mod.version = null_module_version;
  main_mod.status = TULIP_MODULE_UNCOMPILED;
  main_mod.num_definitions = o.main_module.num_definitions;
  main_mod.definitions = malloc(sizeof(tulip_runtime_toplevel_definition) * main_mod.num_definitions);

  for (unsigned int i = 0; i < main_mod.num_definitions; i++) {
    main_mod.definitions[i].name = o.main_module.names[i];
    main_mod.definitions[i].definition = o.main_module.definitions[i];
  }

  main_mod.llvm_module = NULL;
  main_mod.module_scope = NULL;
  main_mod.native_defs = NULL;

  module_ref main_mod_loaded = region_insert_module(state->modules, main_mod);

  tulip_runtime_compile_module(state, region_query_module_by_id(state->modules, main_mod_loaded));

  state->main_module = main_mod_loaded;

  LLVMAddModule(state->jit_instance, region_query_module_by_id(state->modules, main_mod_loaded)->llvm_module);
}

void tulip_runtime_call(module_ref module, const char* function) {
  
}

void tulip_runtime_call_main(tulip_runtime_state* state, unsigned int argc, const char** argv) {
  tulip_runtime_module* m = region_query_module_by_id(state->modules, state->main_module);

  char* v_err;

  LLVMBool invalid = LLVMVerifyModule(m->llvm_module, LLVMPrintMessageAction, &v_err);

  // [todo] catch linker errors

  // [todo] create wrapper function to pass in argv

  // [todo] get main function from modules

  LLVMValueRef main_fn;

  LLVMRunFunction(state->jit_instance, main_fn, 0, NULL);
}

void tulip_runtime_stop(tulip_runtime_state* state) {
  free_value_region(state->values);
  free_module_region(state->modules);

  LLVMDisposeExecutionEngine(state->jit_instance);
  LLVMShutdown();

  free(state);
}
