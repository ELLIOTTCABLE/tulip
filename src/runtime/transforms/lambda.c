#include "runtime/transforms/common.h"

#include <alloca.h>
#include <stdio.h>

unsigned int lambda_count = 0;

char* render_lambda_name(char* def_name, tulip_runtime_module* mod) {
  // [pretty] arbitrary allocation
  char* name = alloca(sizeof(char) * 128);
  sprintf(name, "%s:fn:%d", render_qualified_name((tulip_runtime_ast_name){NULL, 0, def_name}, mod), lambda_count);
  lambda_count = lambda_count + 1;
  return name;
}

LLVMValueRef tulip_runtime_transform_lambda(tulip_runtime_module* mod, char* name, tulip_runtime_ast_lambda lambda, tulip_runtime_context ctx) {
  return NULL;
  // if (ctx.level == top) {
  //   // [todo] toplevel lambda
  // } else {
  //   LLVMValueRef lambda_def = LLVMAddFunction(mod->llvm_module, render_lambda_name(name, mod), tulip_lambda_type);

  //   LLVMBasicBlockRef bb = LLVMAppendBasicBlock(lambda_def, "");
  //   LLVMBuilderRef b = LLVMCreateBuilder();
  //   LLVMPositionBuilderAtEnd(b, bb);

  //   LLVMValueRef upper_scope = LLVMConstIntToPtr(LLVMConstInt(LLVMInt64Type(), (long long) ctx.local_scope, false), LLVMPointerType(LLVMVoidType(), 0));
  //   // [todo] how to return this scope object to a closure?
  //   LLVMValueRef scope = LLVMBuildCall(b, mod->native_defs->scope_init, (LLVMValueRef[]){upper_scope}, 1, "");
  //   LLVMValueRef bind = LLVMGetFirstParam(lambda_def);
  //   LLVMBuildCall(b, mod->native_defs->local_scope_set, (LLVMValueRef[]){scope, LLVMConstString(lambda.bind.name, strlen(lambda.bind.name), false)}, 2, "");

  //   // create closure
  //   LLVMPositionBuilderAtEnd(b, ctx.block);

  //   // get function pointer to lambda_def
  //   // LLVMValueRef fn = LLVMBuildCall(b, mod->native_defs->build_fnptr, (LLVMValueRef[]){lambda_def});
  // }
}
