#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Core.h>

#include <runtime/modules.h>
#include <runtime/state.h>

tulip_runtime_toplevel_definition runtime_wrap_builtin(char* fn_name, unsigned int name_length, unsigned int arity);

LLVMModuleRef runtime_create_builtins();

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

  LLVMValueRef free_tag;
  LLVMValueRef free_string;
  LLVMValueRef free_int;
  LLVMValueRef free_float;
  LLVMValueRef free_closure;
  LLVMValueRef free_fnptr;
} runtime_native_defs;

runtime_native_defs runtime_create_native_decls(tulip_runtime_module* mod);
