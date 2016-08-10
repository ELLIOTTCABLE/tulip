// tulip/llvm transforms
// this file defines a set of mutually recursive functions that transform tulip-core tree to LLVM IR

#include "runtime/transforms.h"

#include <stdlib.h>
#include "types/core.h"
#include "types/value.h"

typedef enum {
  toplevel,
  local
} tulip_runtime_context_level;

////////////////////////////////////////////////////////////////////////////////
// type inference

LLVMTypeRef tulip_runtime_infer_type(tulip_runtime_ast_value* subject) {}

LLVMTypeRef tulip_runtime_infer_value_type(tulip_runtime_ast_value* function) {}

LLVMTypeRef tulip_runtime_infer_function_type(tulip_runtime_ast_value* function) {
  // [todo] recurse down asts, infer their arity and provide scaffolding for type specialization in the future
}

////////////////////////////////////////////////////////////////////////////////
// specific transforms

LLVMValueRef tulip_runtime_transform_name(LLVMModuleRef mod, tulip_runtime_ast_name name, tulip_runtime_context_level ctx) {}
LLVMValueRef tulip_runtime_transform_literal(LLVMModuleRef mod, tulip_runtime_ast_literal lit) {}
LLVMValueRef tulip_runtime_transform_apply(LLVMModuleRef mod, tulip_runtime_ast_apply apply) {}
LLVMValueRef tulip_runtime_transform_block(LLVMModuleRef mod, tulip_runtime_ast_block block) {}
LLVMValueRef tulip_runtime_transform_lambda(LLVMModuleRef mod, tulip_runtime_ast_lambda lambda) {}
LLVMValueRef tulip_runtime_transform_let(LLVMModuleRef mod, tulip_runtime_ast_let let) {}
LLVMValueRef tulip_runtime_transform_tag(LLVMModuleRef mod, tulip_runtime_ast_tag tag) {}
LLVMValueRef tulip_runtime_transform_builtin(LLVMModuleRef mod, tulip_runtime_ast_builtin builtin) {}
LLVMValueRef tulip_runtime_transform_branch(LLVMModuleRef mod, tulip_runtime_ast_branch branch) {}

////////////////////////////////////////////////////////////////////////////////
// dispatch and interface

// dispatch to actual builders
LLVMValueRef tulip_runtime_transform_core(LLVMModuleRef mod, tulip_runtime_ast_value* ast, bool is_toplevel) {
  switch (ast->type) {
  case ast_name:
    return tulip_runtime_transform_name(mod, ast->name, is_toplevel);
  case ast_literal:
    return tulip_runtime_transform_literal(mod, ast->literal);
  case ast_tag:
    return tulip_runtime_transform_tag(mod, ast->tag);
  case ast_apply:
    return tulip_runtime_transform_apply(mod, ast->apply);
  case ast_block:
    return tulip_runtime_transform_block(mod, ast->block);
  case ast_lambda:
    return tulip_runtime_transform_lambda(mod, ast->lambda);
  case ast_let:
    return tulip_runtime_transform_let(mod, ast->let);
  case ast_branch:
    return tulip_runtime_transform_branch(mod, ast->branch);
  case ast_builtin:
    return tulip_runtime_transform_builtin(mod, ast->builtin);
  }
}

// compiles all top-level definitions into ir
void tulip_runtime_compile_module(tulip_runtime_module* mod) {
  if (mod->status == TULIP_MODULE_UNCOMPILED || mod->status == TULIP_MODULE_MODIFIED) {
    mod->llvm_module = LLVMModuleCreateWithName(mod->name);

    for (int i = 0; i < mod->num_definitions; i++) {
      char* def_name = mod->definitions[i].name;
      tulip_runtime_ast_value* ast = convert_tag_to_ast_value(mod->definitions[i].definition);
      LLVMTypeRef def_ty = tulip_runtime_infer_type(ast);
      LLVMValueRef entry = tulip_runtime_transform_core(mod->llvm_module, ast, toplevel);

      mod->definitions[i].corresponding_llvm_definition =
        (tulip_runtime_toplevel_definition_compiled) {
        .llvm_function_name = def_name,
        .llvm_function_type = def_ty,
        .llvm_entry_point   = entry
      };
    }
  }
}

