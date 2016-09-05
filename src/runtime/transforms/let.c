#include "runtime/transforms/common.h"

LLVMValueRef tulip_runtime_transform_let(tulip_runtime_module* mod, char* name, tulip_runtime_ast_let let, tulip_runtime_context ctx) {
  // append to context basic block
  //   evaluate rhs
  //   evalute name_set(lhs, rhs_result)
}
