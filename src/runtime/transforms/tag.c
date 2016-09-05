
#include "runtime/transforms/common.h"

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
