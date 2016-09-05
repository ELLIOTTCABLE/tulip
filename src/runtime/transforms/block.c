#include "runtime/transforms/common.h"

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
