#include "runtime/transforms/common.h"

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
