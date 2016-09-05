// transforms
// this file defines a set of mutually recursive functions that transform tulip-core tree to LLVM IR

#pragma once

#include "runtime/transforms/common.h"

void tulip_runtime_compile_module(tulip_runtime_module* mod);

LLVMValueRef tulip_runtime_transform_name_get(tulip_runtime_module* mod, char* def_name, tulip_runtime_ast_name name, tulip_runtime_context ctx);
//LLVMValueRef tulip_runtime_transform_name_set();
LLVMValueRef tulip_runtime_transform_tag(tulip_runtime_module* mod, char* def_name, tulip_runtime_ast_tag tag, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_block(tulip_runtime_module* mod, char* name, tulip_runtime_ast_block block, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_literal(tulip_runtime_module* mod, char* def_name, tulip_runtime_ast_literal lit, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_apply(tulip_runtime_module* mode, char* name, tulip_runtime_ast_apply apply, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_lambda(tulip_runtime_module* mod, char* name, tulip_runtime_ast_lambda lambda, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_let(tulip_runtime_module* mod, char* name, tulip_runtime_ast_let let, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_builtin(tulip_runtime_module* mod, char* name, tulip_runtime_ast_builtin builtin, tulip_runtime_context ctx);
LLVMValueRef tulip_runtime_transform_branch(tulip_runtime_module* mod, char* name, tulip_runtime_ast_branch branch, tulip_runtime_context ctx);
