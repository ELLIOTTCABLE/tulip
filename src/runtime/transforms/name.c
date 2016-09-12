#include "runtime/transforms/common.h"

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
      LLVMBuildCall(b, extern_fn, NULL, 0, "");

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
