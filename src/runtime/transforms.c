// tulip/llvm transforms
// this file defines a set of mutually recursive functions that transform tulip-core tree to LLVM IR

#include "runtime/transforms.h"

#include <stdlib.h>
#include <string.h>

#include "types/core.h"
#include "types/value.h"
#include "support/constants.h"
#include "runtime/builtins.h"


  typedef struct tulip_runtime_context {
  enum { top, lower } level;
  tulip_runtime_scope* local_scope; // i64 containing a pointer to a scope struct
  LLVMBasicBlockRef block;
} tulip_runtime_context;

LLVMValueRef ctx_get_scope_ptr(LLVMBuilderRef b, tulip_runtime_context ctx) {
  LLVMValueRef ptr_int = LLVMConstInt(LLVMInt64Type(), (unsigned long long) ctx.local_scope, false);
  LLVMValueRef ptr = LLVMBuildIntToPtr(b, ptr_int, LLVMPointerType(LLVMVoidType(), 0), "");
  return ptr;
}

LLVMValueRef tulip_runtime_transform_core(tulip_runtime_module* mod, char* name, tulip_runtime_ast_value* ast, tulip_runtime_context ctx);

////////////////////////////////////////////////////////////////////////////////
// specific transforms

// [todo] these transforms are pretty fundamentally big, and need to be carefully documented
//        they should probably have their own individual files

LLVMValueRef tulip_runtime_transform_name_get(tulip_runtime_module* mod, char* def_name, tulip_runtime_ast_name name, tulip_runtime_context ctx) {
  if (ctx.level == top) {

    // definition bound to a single name
    // eg. `imported-val = Other/Mod/value`

    if (name.modulePathLen > 0) {

      // this name is fully qualified and needs to link with another module
      // it is currently the compiler's responsibility to resolve linkage conflicts, the runtime only reports linker errors

      // create a function prototype corresponding to the external module reference
      LLVMValueRef extern_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name(name, mod), tulip_defn_type);

      // create another function corresponding to the local binding
      LLVMValueRef local_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name((tulip_runtime_ast_name) {NULL, 0, def_name}, mod), tulip_defn_type);

      // build a call to the function prototype in the local binding function
      LLVMBasicBlockRef bb = LLVMAppendBasicBlock(local_fn, "");
      LLVMBuilderRef b = LLVMCreateBuilder();
      LLVMPositionBuilderAtEnd(b, bb);

      // [note] the string paramater in this call is used by the optimizer i think, but isn't semantically relevant
      LLVMBuildCall(b, extern_fn, NULL, 0, "extcall");

      // [todo] should probably just but a builder in the context struct and reuse it
      //        verify that it only tracks position state
      LLVMDisposeBuilder(b);

      // return local binding as the definition rhs
      return local_fn;

    } else {

      // this name is local and can be located in, or bound to, the scope chain

      LLVMValueRef local_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name((tulip_runtime_ast_name){NULL, 0, def_name}, mod), tulip_defn_type);

      LLVMBasicBlockRef bb = LLVMAppendBasicBlock(local_fn, "");
      LLVMBuilderRef b = LLVMCreateBuilder();
      LLVMPositionBuilderAtEnd(b, bb);

      // retrieve name from scope object
      LLVMValueRef str = LLVMConstString(name.name, strlen(name.name), false);
      LLVMValueRef ptr = ctx_get_scope_ptr(b, ctx);
      LLVMValueRef ret = LLVMBuildCall(b, mod->native_defs->local_scope_lookup, (LLVMValueRef[]){ptr, str}, 2, "");
      LLVMBuildRet(b, ret);

      LLVMDisposeBuilder(b);

      return local_fn;
    }

  } else {

    // name in a callsite context
    // eg. `a_binding = { Other/Mod/this_name .nil }`

    if (name.modulePathLen > 0) {

      // external linkage, same as above but not local function is generated

      LLVMValueRef extern_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name(name, mod), tulip_defn_type);

      LLVMBuilderRef b = LLVMCreateBuilder();
      LLVMPositionBuilderAtEnd(b, ctx.block);
      LLVMValueRef ret = LLVMBuildCall(b, extern_fn, NULL, 0, "extcall");

      LLVMDisposeBuilder(b);

      return ret;

    } else {

      // scope reference

      LLVMBuilderRef b = LLVMCreateBuilder();
      LLVMPositionBuilderAtEnd(b, ctx.block);

      LLVMValueRef str = LLVMConstString(name.name, strlen(name.name), false);
      LLVMValueRef ptr = ctx_get_scope_ptr(b, ctx);
      LLVMValueRef ret = LLVMBuildCall(b, mod->native_defs->local_scope_lookup, (LLVMValueRef[]){ptr, str}, 2, "");

      LLVMDisposeBuilder(b);

      return ret;
    }
  }
}

// [todo] name assignment semantics
//        implement along with let
//LLVMValueRef tulip_runtime_transform_name_set() {}

LLVMValueRef tulip_runtime_transform_literal(tulip_runtime_module* mod, char* def_name, tulip_runtime_ast_literal lit, tulip_runtime_context ctx) {
  if (ctx.level == top) {

    LLVMValueRef local_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name((tulip_runtime_ast_name){NULL, 0, def_name}, mod), tulip_defn_type);

    LLVMBasicBlockRef bb = LLVMAppendBasicBlock(local_fn, "");

    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, bb);

    LLVMValueRef ret;

    switch (lit.type) {

    case ast_literal_int:
      ret = LLVMConstInt(LLVMInt64Type(), (unsigned long long) lit.integral, true);

    case ast_literal_float:
      ret = LLVMConstReal(LLVMDoubleType(), lit.fractional);

    case ast_literal_string:
      ret = LLVMConstString(lit.string, strlen(lit.string), false);
    }

    LLVMBuildRet(b, ret);

    LLVMDisposeBuilder(b);

    return local_fn;

  } else {

    switch (lit.type) {

    case ast_literal_int:
      return LLVMConstInt(LLVMInt64Type(), (unsigned long long) lit.integral, true);

    case ast_literal_float:
      return LLVMConstReal(LLVMDoubleType(), lit.fractional);

    case ast_literal_string:
      return LLVMConstString(lit.string, strlen(lit.string), false);
    }

  }
}
LLVMValueRef tulip_runtime_transform_apply(LLVMModuleRef mod, tulip_runtime_ast_apply apply, tulip_runtime_context ctx) {}

LLVMValueRef tulip_runtime_transform_block(tulip_runtime_module* mod, char* name, tulip_runtime_ast_block block, tulip_runtime_context ctx) {
  if(ctx.level == top) {
    // [?] without "module effects", is this an error case or should i wrap blocks in local defs
    return NULL;
  } else {

    LLVMBasicBlockRef bb = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(ctx.block), "");

    tulip_runtime_context content_ctx = ctx;
    content_ctx.block = bb;

    LLVMValueRef ret;

    for (unsigned int i = 0; i < block.length; i++) {
      ret = tulip_runtime_transform_core(mod, name, block.statements[i], content_ctx);
    }

    return ret;
  }
}

LLVMValueRef tulip_runtime_transform_lambda(LLVMModuleRef mod, tulip_runtime_ast_lambda lambda, tulip_runtime_context ctx) {}

LLVMValueRef tulip_runtime_transform_let(LLVMModuleRef mod, tulip_runtime_ast_let let, tulip_runtime_context ctx) {
  // append to context basic block
  //   evaluate rhs
  //   evalute name_set(lhs, rhs_result)
}

LLVMValueRef tulip_runtime_transform_tag(tulip_runtime_module* mod, char* def_name, tulip_runtime_ast_tag tag, tulip_runtime_context ctx) {
  if(ctx.level == top) {

    LLVMValueRef local_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name((tulip_runtime_ast_name){NULL, 0, def_name}, mod), tulip_defn_type);
    LLVMBasicBlockRef bb = LLVMAppendBasicBlock(local_fn, "");

    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, bb);

    ctx.block = bb;

    LLVMValueRef* contents = malloc(sizeof(LLVMValueRef) * tag.contents_length);

    for (unsigned int i = 0; i < tag.contents_length; i++) {
      contents[i] = tulip_runtime_transform_core(mod, def_name, tag.contents[i], ctx);
    }

    LLVMValueRef length = LLVMConstInt(LLVMInt64Type(), (unsigned long long) tag.contents_length, false);
    LLVMValueRef str = LLVMConstString(tag.tag_name, strlen(tag.tag_name), false);
    LLVMValueRef array = LLVMConstArray(tulip_value_type, contents, tag.contents_length);
    LLVMValueRef ret = LLVMBuildCall(b, mod->native_defs->build_tag, (LLVMValueRef[]){str, length, array}, 3, "");
    LLVMBuildRet(b, ret);

    LLVMDisposeBuilder(b);

    return local_fn;

  } else {

    LLVMValueRef* contents = malloc(sizeof(LLVMValueRef) * tag.contents_length);

    for (unsigned int i = 0; i < tag.contents_length; i++) {
      contents[i] = tulip_runtime_transform_core(mod, def_name, tag.contents[i], ctx);
    }

    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, ctx.block);

    LLVMValueRef length = LLVMConstInt(LLVMInt64Type(), (unsigned long long) tag.contents_length, false);
    LLVMValueRef str = LLVMConstString(tag.tag_name, strlen(tag.tag_name), false);
    LLVMValueRef array = LLVMConstArray(tulip_value_type, contents, tag.contents_length);
    LLVMValueRef ret = LLVMBuildCall(b, mod->native_defs->build_tag, (LLVMValueRef[]){str, length, array}, 3, "");

    LLVMDisposeBuilder(b);

    return ret;
  }
}

LLVMValueRef tulip_runtime_transform_builtin(LLVMModuleRef mod, tulip_runtime_ast_builtin builtin, tulip_runtime_context ctx) {}
LLVMValueRef tulip_runtime_transform_branch(LLVMModuleRef mod, tulip_runtime_ast_branch branch, tulip_runtime_context ctx) {}

////////////////////////////////////////////////////////////////////////////////
// dispatch and interface

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
    return tulip_runtime_transform_apply(mod, ast->apply, ctx);
  case ast_block:
    return tulip_runtime_transform_block(mod, name, ast->block, ctx);
  case ast_lambda:
    return tulip_runtime_transform_lambda(mod, ast->lambda, ctx);
  case ast_let:
    return tulip_runtime_transform_let(mod, ast->let, ctx);
  case ast_branch:
    return tulip_runtime_transform_branch(mod, ast->branch, ctx);
  case ast_builtin:
    return tulip_runtime_transform_builtin(mod, ast->builtin, ctx);
  }
}

// compiles all top-level definitions into ir
void tulip_runtime_compile_module(tulip_runtime_module* mod) {
  if (mod->status == TULIP_MODULE_UNCOMPILED || mod->status == TULIP_MODULE_COMPILED) {
    mod->llvm_module = LLVMModuleCreateWithName(mod->name);

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

