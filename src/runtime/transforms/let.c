#include "runtime/transforms/common.h"

#include <assert.h>

LLVMValueRef tulip_runtime_transform_let(tulip_runtime_module* mod, char* name, tulip_runtime_ast_let let, tulip_runtime_context ctx) {

    // [note] this case is almost certainly not valid
    assert(ctx.level != top);

    LLVMValueRef rhs = tulip_runtime_transform_core(mod, name, let.definition, ctx);

    LLVMValueRef scope = LLVMConstIntToPtr(LLVMConstInt(LLVMInt64Type(), (long long) ctx.local_scope, false), LLVMPointerType(LLVMVoidType(), 0));

    LLVMBuilderRef b = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(b, ctx.block);

    if (let.bind.modulePathLen > 0) {
      // [note] what to do for assignment to qualified names?
    } else {
      LLVMBuildCall(b, mod->native_defs->local_scope_set, (LLVMValueRef[]){scope, LLVMConstString(let.bind.name, strlen(let.bind.name), false)}, 2, "");
    }

    // [note] lets are void and consuming their value is always invalid
    LLVMValueRef poison = LLVMGetUndef(tulip_value_type);

    return poison;
}
