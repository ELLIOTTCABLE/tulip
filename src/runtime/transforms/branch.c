#include "runtime/transforms/common.h"

LLVMValueRef tulip_runtime_transform_branch(tulip_runtime_module* mod, char* name, tulip_runtime_ast_branch branch, tulip_runtime_context ctx) {
  if(ctx.level == top) {
  } else {
    tulip_runtime_context branch_ctx = ctx;
    // [note] this allocates a scope object for each branch, which never gets freed
    //        this is really not that wasteful since the scopes are completely empty until branches are evaluated
    //        but is still messy and should be rethought
    branch_ctx.local_scope = scope_init(ctx.local_scope);

    LLVMBasicBlockRef top_bb = ctx.block;
    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, top_bb);

    LLVMValueRef scope_ptr = LLVMConstIntToPtr(LLVMConstInt(LLVMInt64Type(), (long long) branch_ctx.local_scope, false), LLVMPointerType(LLVMVoidType(), 0));

    for (unsigned int i = 0; i < branch.length; i++) {
      LLVMValueRef predicate = tulip_runtime_transform_core(mod, name, branch.predicates[i], branch_ctx);

      // pattern success block

      LLVMBasicBlockRef pass_bb = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(ctx.block), "");
      LLVMPositionBuilderAtEnd(b, pass_bb);

      tulip_runtime_context pass_ctx = branch_ctx;
      pass_ctx.block = pass_bb;
      LLVMValueRef consequence = tulip_runtime_transform_core(mod, name, branch.consequences[i], pass_ctx);

      // [todo] don't run other predicates

      LLVMBuildCall(b, mod->native_defs->pattern_scope_reset, (LLVMValueRef[]){scope_ptr}, 1, "");

      // pattern fail block

      LLVMBasicBlockRef fail_bb = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(ctx.block), "");
      LLVMPositionBuilderAtEnd(b, fail_bb);

      LLVMBuildCall(b, mod->native_defs->pattern_scope_reset, (LLVMValueRef[]){scope_ptr}, 1, "");

      // branch

      LLVMPositionBuilderAtEnd(b, top_bb);

      LLVMValueRef is_t = LLVMBuildCall(b, mod->native_defs->test_boolean, (LLVMValueRef[]){predicate}, 1, "");
      LLVMBuildCondBr(b, is_t, pass_bb, fail_bb);
    }
  }
}
