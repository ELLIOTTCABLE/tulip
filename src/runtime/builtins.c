#include "runtime/builtins.h"

#include <alloca.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <llvm-c/Types.h>
#include <llvm-c/ExecutionEngine.h>

#include "runtime/modules.h"
#include "runtime/state.h"
#include "runtime/values.h"
#include "types/core.h"
#include "support/constants.h"

////////////////////////////////////////////////////////////////////////////////
// builtin functions
// builtins that are placed in the builtin prelude module are named builtin_*
// builtins that are placed in front of every module are named native_*
// should probably be moved somewhere else

// these are externed to guarantee they'll be available in the jitted code's address space
// this may not be necessary but is probably a simpler solution than writing an address remapper

extern void builtin_print(tulip_runtime_value* s) {}

// native constructors
extern value_ref native_build_tag(const char* tag, unsigned int length, LLVMValueRef* contents) {

}
extern value_ref native_build_string() {}
extern value_ref native_build_int() {}
extern value_ref native_build_float() {}
extern value_ref native_build_closure() {}
extern value_ref native_build_fnptr() {}

// native value inspection
extern char* native_inspect_type(tulip_runtime_value* v) {}

extern value_ref native_local_scope_lookup(void* scope, const char* name) {
  tulip_runtime_scope* s = (tulip_runtime_scope*) scope;

  value_ref ret = scope_lookup(s, name);

  if (ret) {
    return ret;
  } else {
    // [todo] crash process
    return (value_ref) NULL;
  }
}

extern void native_local_scope_set(void* scope, const char* name, void* value) {
  tulip_runtime_scope* s = (tulip_runtime_scope*) scope;
  value_ref v = (value_ref) value;

  scope_insert(s, name, v);

  return;
}

// native destructors
extern void native_free_tag(tulip_runtime_value* v) {}
extern void native_free_string(tulip_runtime_value* v) {}
extern void native_free_int(tulip_runtime_value* v) {}
extern void native_free_float(tulip_runtime_value* v) {}
extern void native_free_closure(tulip_runtime_value* v) {}
extern void native_free_fnptr(tulip_runtime_value* v) {}

////////////////////////////////////////////////////////////////////////////////
// builtin utilities

// constructs a tulip definition around a c function
tulip_runtime_toplevel_definition runtime_wrap_builtin(char* fn_name, unsigned int name_length, unsigned int arity) {
  tulip_value names[arity];

  char* builtin_name = alloca(sizeof(char) * (name_length + 8));
  strcpy(builtin_name, "builtin_");
  strcat(builtin_name, fn_name); // [note] strcat may not behave well

  tulip_value bottom = builtin(builtin_name, arity, (tulip_value[]){}, 0);

  tulip_value* cur = &bottom;
  for (unsigned int i = 0; i < arity; i++) {
    tulip_value prev = *cur;
    char* param_name = (char[]){'p', i + 48};
    names[i] = name(NULL, 0, param_name);
    tulip_value next = lambda(names[i], prev);
    cur = &next;
  }

  return (tulip_runtime_toplevel_definition) { name, *cur, NULL };
}

void runtime_create_builtins_binding (tulip_runtime_module* mod, tulip_runtime_state* state) {
  LLVMValueRef print = LLVMAddFunction(mod->llvm_module, "builtins_print", tulip_defn_type);
  LLVMAddGlobalMapping(state->jit_instance, print, &builtin_print);
  // TODO [sig] resolve this
  // module_insert_definition(mod, runtime_wrap_builtin("print", 5, 1));
}

tulip_runtime_module* runtime_create_builtins_module(tulip_runtime_state* state) {
  tulip_runtime_module* tulip_mod = malloc(sizeof(tulip_runtime_module));
  tulip_mod->name = "builtins";
  tulip_mod->path_len = 1;
  tulip_mod->path = malloc(sizeof(char*));
  tulip_mod->path[0] = "prelude";
  tulip_mod->version = (tulip_runtime_module_version){"generated", 1, 0};
  tulip_mod->status = TULIP_MODULE_EMPTY;
  tulip_mod->num_definitions = 0;
  tulip_mod->llvm_module = LLVMModuleCreateWithName(tulip_mod->name);

  runtime_create_builtins_binding(tulip_mod, state);

  tulip_mod->status = TULIP_MODULE_COMPILED; // to preserve the external function decls, we need to drive compilation explicitly here
  return tulip_mod;
}

runtime_native_defs runtime_create_native_decls(tulip_runtime_module* mod) {
  LLVMAddFunction(mod->llvm_module, "native_build_tag", tulip_defn_type);
}
