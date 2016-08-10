#include "runtime/builtins.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <llvm-c/Types.h>
#include <llvm-c/ExecutionEngine.h>

#include "runtime/state.h"
#include "types/core.h"

extern void builtin_print(char* s) {
  printf("%s\n", s);
}

tulip_runtime_toplevel_definition runtime_wrap_builtin(char* fn_name, unsigned int name_length, unsigned int arity, void* function, tulip_runtime_state* LLVM) {
  tulip_value names[arity];

  char* builtin_name = alloca(sizeof(char) * (name_length + 8));
  strcpy(builtin_name, "builtin_");
  strcat(builtin_name, fn_name);

  tulip_value bottom = builtin(builtin_name, arity, (tulip_value[]){}, 0);

  tulip_value* cur = &bottom;
  for (int i = 0; i < arity; i++) {
    tulip_value prev = *cur;
    char* param_name = (char[]){'p', i + 48};
    names[i] = name(param_name);
    tulip_value next = lambda(names[i], prev);
    cur = &next;
  }

  return (tulip_runtime_toplevel_definition) { name, *cur, NULL };
}

LLVMModuleRef runtime_create_builtins_binding (tulip_runtime_state* state) {
  LLVMModuleRef mod = LLVMModuleCreateWithName("builtin");
  LLVMValueRef print = LLVMAddFunction(mod, "builtin_print", LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, false));
  LLVMAddGlobalMapping(state->jit_instance, print, &builtin_print);

  return mod;
}

tulip_runtime_module runtime_create_builtins_module(tulip_runtime_state* state) {
  
}
