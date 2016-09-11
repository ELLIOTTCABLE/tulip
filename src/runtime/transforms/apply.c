#include "runtime/transforms/common.h"

LLVMValueRef tulip_runtime_transform_apply(tulip_runtime_module* mod, char* name, tulip_runtime_ast_apply apply, tulip_runtime_context ctx) {
  if (ctx.level == top) {
    // [todo] toplevel apply
  } else {

    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, ctx.block);

    LLVMValueRef call_position;
    LLVMValueRef arg_position;

    if (apply.call->type == ast_name) {
      if (apply.call->name.modulePathLen > 0) {
        char* name = render_qualified_name(apply.call->name, mod);
        // [todo] retrive module definition
        //        call to retrive lambda
        // LLVMBuildCall(b, )
      } else {
        LLVMValueRef name = LLVMConstString(apply.call->name.name, strlen(apply.call->name.name), false);
        LLVMValueRef scope = LLVMConstIntToPtr(LLVMConstInt(LLVMInt64Type(), (long long) ctx.local_scope, false), LLVMPointerType(LLVMVoidType(), 0));
        call_position = LLVMBuildCall(b, mod->native_defs->local_scope_lookup, (LLVMValueRef[]){scope, name}, 2, "");
      }
    } else {
      call_position = tulip_runtime_transform_core(mod, name, apply.call, ctx);
    }


    for (unsigned int i = 0; i < apply.saturation; i++) {
      // strictly evaluate argument position

      arg_position = tulip_runtime_transform_core(mod, name, apply.arguments[i], ctx);

      // [todo] guard against malformed calls here
      // [todo] retrive value of call_position from values region, make sure it's a fnptr or closure
      LLVMValueRef callee;

      call_position = LLVMBuildCall(b, callee, (LLVMValueRef[]){arg_position}, 1, "");

    }

    LLVMDisposeBuilder(b);

    return call_position;

  }
}

// [todo] builtin application
