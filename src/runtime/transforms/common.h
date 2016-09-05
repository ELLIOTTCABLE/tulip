#pragma once

#include <stdlib.h>
#include <string.h>

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Core.h>

#include "runtime/ast.h"
#include "runtime/modules.h"
#include "runtime/builtins.h"
#include "types/core.h"
#include "types/value.h"
#include "support/constants.h"

typedef struct tulip_runtime_context {
  enum { top, lower } level;
  tulip_runtime_scope* local_scope; // i64 containing a pointer to a scope struct
  LLVMBasicBlockRef block;
} tulip_runtime_context;

// declared here for use by recursive transforms
LLVMValueRef tulip_runtime_transform_core(tulip_runtime_module* mod, char* name, tulip_runtime_ast_value* ast, tulip_runtime_context ctx);

LLVMValueRef ctx_get_scope_ptr(LLVMBuilderRef b, tulip_runtime_context ctx);
