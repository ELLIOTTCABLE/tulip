#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Core.h>

#include <runtime/modules.h>
#include <runtime/state.h>

typedef struct runtime_native_defs {
  LLVMValueRef build_tag;
  LLVMValueRef build_string;
  LLVMValueRef build_int;
  LLVMValueRef build_float;
  LLVMValueRef build_closure;
  LLVMValueRef build_fnptr;

  LLVMValueRef inspect_type;
  LLVMValueRef local_scope_lookup;
  LLVMValueRef local_scope_set;

  LLVMValueRef test_boolean;

  LLVMValueRef free_value;
} runtime_native_defs;

runtime_native_defs runtime_create_native_decls(tulip_runtime_module* mod, tulip_runtime_state* state);
tulip_runtime_module* runtime_create_builtins_module(tulip_runtime_state* state);
