// tulip/llvm transforms
// this file defines a set of mutually recursive functions that transform tulip-core tree to LLVM IR

#include "runtime/transforms.h"

// converts a runtime-managed scope object to a static pointer
LLVMValueRef ctx_get_scope_ptr(LLVMBuilderRef b, tulip_runtime_context ctx) {
  LLVMValueRef ptr_int = LLVMConstInt(LLVMInt64Type(), (unsigned long long) ctx.local_scope, false);
  LLVMValueRef ptr = LLVMBuildIntToPtr(b, ptr_int, LLVMPointerType(LLVMVoidType(), 0), "");
  return ptr;
}

// dispatch to actual builders
LLVMValueRef tulip_runtime_transform_core(tulip_runtime_module* mod, char* name, tulip_runtime_ast_value* ast, tulip_runtime_context ctx) {
  switch (ast->type) {
  case ast_name:
    return tulip_runtime_transform_name_get(mod, name, ast->name, ctx);
  case ast_literal:
    return tulip_runtime_transform_literal(mod, name, ast->literal, ctx);
  case ast_tag:
    return tulip_runtime_transform_tag(mod, name, ast->tag, ctx);
  case ast_apply:
    return tulip_runtime_transform_apply(mod, name, ast->apply, ctx);
  case ast_block:
    return tulip_runtime_transform_block(mod, name, ast->block, ctx);
  case ast_lambda:
    return tulip_runtime_transform_lambda(mod, name, ast->lambda, ctx);
  case ast_let:
    return tulip_runtime_transform_let(mod, name, ast->let, ctx);
  case ast_branch:
    return tulip_runtime_transform_branch(mod, name, ast->branch, ctx);
  case ast_builtin:
    return tulip_runtime_transform_builtin(mod, name, ast->builtin, ctx);
  }
}

// compiles all top-level definitions into ir
void tulip_runtime_compile_module(tulip_runtime_state* state, tulip_runtime_module* mod) {
  if (mod->status == TULIP_MODULE_UNCOMPILED || mod->status == TULIP_MODULE_COMPILED) {
    mod->llvm_module = LLVMModuleCreateWithName(mod->name);

    mod->module_scope = scope_init(NULL);
    mod->native_defs = runtime_create_native_decls(mod, state);

    for (unsigned int i = 0; i < mod->num_definitions; i++) {
      char* bind_name = mod->definitions[i].name;
      tulip_runtime_ast_value* ast = convert_tag_to_ast_value(mod->definitions[i].definition);

      tulip_runtime_context ctx;
      ctx.level = top;
      ctx.local_scope = mod->module_scope;

      LLVMValueRef entry = tulip_runtime_transform_core(mod, bind_name, ast, ctx);

      tulip_runtime_ast_name def_name;
      def_name.name = bind_name;

      mod->definitions[i].corresponding_llvm_definition =
        (tulip_runtime_toplevel_definition_compiled) {
        .llvm_function_name = render_qualified_name(def_name, mod),
        .llvm_entry_point   = entry
      };
    }
  }
}

