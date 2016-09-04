// tulip modules
// this file defines the structure of a tulip module at runtime and provides some utility functions for working with them

#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <stdlib.h>
#include <llvm-c/Core.h>

#include "types/value.h"

// [todo] [urgent] work out this ridiculous import cycle
struct tulip_runtime_scope;
struct runtime_native_defs;

typedef struct {
  char* llvm_function_name;
  LLVMValueRef llvm_entry_point;
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
  TULIP_MODULE_TRANSFORMED,
  TULIP_MODULE_COMPILED
} tulip_runtime_module_status;

typedef struct {
  char* name;
  char** path;
  unsigned int path_len;
  tulip_runtime_module_version version;
  tulip_runtime_module_status status;
  tulip_runtime_toplevel_definition* definitions;
  unsigned int num_definitions;
  LLVMModuleRef llvm_module;
  struct tulip_runtime_scope* module_scope;
  struct runtime_native_defs* native_defs; // [note] every module should contain its own native header
                                           //        it's a little wasteful, but it allows llvm to inline the foreign calls
} tulip_runtime_module;

void module_insert_definition(tulip_runtime_module* mod, tulip_runtime_toplevel_definition def);
bool module_contains_definition_named(const char* name);
