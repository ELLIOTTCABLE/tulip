#include "runtime/transforms/common.h"

// [todo] block introduces a scope

LLVMValueRef tulip_runtime_transform_block(tulip_runtime_module* mod, char* name, tulip_runtime_ast_block block, tulip_runtime_context ctx) {
  if(ctx.level == top) {

    LLVMValueRef local_fn = LLVMAddFunction(mod->llvm_module, render_qualified_name((tulip_runtime_ast_name){NULL, 0, name}, mod), tulip_defn_type);

    LLVMBasicBlockRef bb = LLVMAppendBasicBlock(local_fn, "");

    tulip_runtime_context content_ctx = ctx;
    content_ctx.block = bb;
    content_ctx.local_scope = scope_init(ctx.local_scope);

    // [todo] scope destruction at runtime

    LLVMValueRef ret;

    for (unsigned int i = 0; i < block.length; i++) {
      ret = tulip_runtime_transform_core(mod, name, block.statements[i], content_ctx);
    }

    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, bb);

    LLVMBuildRet(b, ret);

    LLVMDisposeBuilder(b);

    return local_fn;

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
