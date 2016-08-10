// tulip modules
// this file defines the structure of a tulip module at runtime and provides some utility functions for working with them

#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <stdlib.h>
#include <llvm-c/Core.h>

#include "types/value.h"

typedef struct {
  char* llvm_function_name;
  LLVMTypeRef llvm_function_type;
  LLVMValueRef llvm_entry_point; // points to a function
} tulip_runtime_toplevel_definition_compiled;

typedef struct {
  char* name;
  tulip_value definition;
  tulip_runtime_toplevel_definition_compiled corresponding_llvm_definition;
} tulip_runtime_toplevel_definition;

typedef struct {
  const char* user_version;
  unsigned int build_version;
  unsigned int source_id;
} tulip_runtime_module_version;

const tulip_runtime_module_version null_module_version;

typedef enum {
  TULIP_MODULE_EMPTY,
  TULIP_MODULE_UNCOMPILED,
  TULIP_MODULE_COMPILED,
  TULIP_MODULE_MODIFIED
} tulip_runtime_module_status;

// [care] modules will need to be managed by the process collector, and should not be allocated on their own
typedef struct {
  char* name;
  tulip_runtime_module_version version;
  tulip_runtime_module_status status;
  tulip_runtime_toplevel_definition* definitions;
  int num_definitions;
  LLVMModuleRef llvm_module;
} tulip_runtime_module;

const tulip_runtime_module main_module;
