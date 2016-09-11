#include "runtime/builtins.h"

#include <alloca.h>
#include <assert.h>
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
// should probably be moved somewhere else

// these are externed to guarantee they'll be available in the jitted code's address space
// this may not be necessary but is probably a simpler solution than writing an address remapper

extern void builtin_print(tulip_runtime_value* s) {}

////////////////////////////////////////////////////////////////////////////////
// native functions
// wrappers for these functions are placed in every tulip module
// they will eventually be rewritten as llvm transforms

// [todo] thread region pointer to these native calls
//        the best way to do this may actually just be a global var

// runtime value constructors
extern value_ref native_build_tag(const char* tag, unsigned int length, value_ref* contents) {
  tulip_runtime_value v;
  v.type = tulip_value_tag;
  v.tag = (tulip_runtime_tag){tag, length, contents};

  return region_insert_value(NULL, v);
}

extern value_ref native_build_string(const char* value) {
  tulip_runtime_value v = {.type = tulip_value_string, .string = (tulip_runtime_string) {strlen(value), value}};

  return region_insert_value(NULL, v);
}

extern value_ref native_build_int(long long value) {
  tulip_runtime_value v = {.type = tulip_value_integral, .integral = (tulip_runtime_integral){value}};

  return region_insert_value(NULL, v);
}

extern value_ref native_build_float(double value) {
  tulip_runtime_value v = {.type = tulip_value_fractional, .fractional = (tulip_runtime_fractional){value}};

  return region_insert_value(NULL, v);
}

// constructs a closure with an empty scope
extern value_ref native_build_closure(value_ref fn) {
  // [todo] thread scope parent
  // [note] be careful about lexical scoping rules here
  tulip_runtime_scope* s = scope_init(NULL);
  tulip_runtime_value v;
  v.type = tulip_value_closure;
  v.closure = (tulip_runtime_closure){s, fn};

  // [todo] see above
  return region_insert_value(NULL, v);
}

extern value_ref native_build_fnptr(void* fn) {
  tulip_runtime_fnptr fnptr = (tulip_runtime_fnptr) fn;
  tulip_runtime_value v;
  v.type = tulip_value_fnptr;
  v.fnptr = fnptr;

  return region_insert_value(NULL, v);
}

// native value inspection
extern char* native_inspect_type(value_ref v) {
  tulip_runtime_value* val = region_get_value(NULL, v);

  switch (val->type) {
  case tulip_value_tag:
    return "tag";
  case tulip_value_string:
    return "string";
  case tulip_value_integral:
    return "integral";
  case tulip_value_fractional:
    return "fractional";
  case tulip_value_closure:
    return "closure";
  case tulip_value_fnptr:
    return "fnptr";
  }
}

extern value_ref native_local_scope_lookup(void* scope, const char* name) {
  tulip_runtime_scope* s = (tulip_runtime_scope*) scope;

  value_ref ret = scope_lookup(s, name);

  if (ret) {
    return ret;
  } else {
    assert(false);
    return (value_ref) NULL;
  }
}

extern void native_local_scope_set(void* scope, const char* name, void* value) {
  tulip_runtime_scope* s = (tulip_runtime_scope*) scope;
  value_ref v = (value_ref) value;

  scope_insert(s, name, v);

  return;
}

extern bool native_test_boolean(value_ref v) {
  tulip_runtime_value* val = region_get_value(NULL, v);
  switch (val->type) {
  case tulip_value_tag:
    if (strcmp(val->tag.name, "t") == 0) {
      return true;
    } else if (strcmp(val->tag.name, "f") == 0) {
      return false;
    }
  default:
    // [todo] crash process gracefully
    assert(false);
    break;
  }
}

// native destructors
extern void native_free_value(int value) {
  value_ref v = (value_ref) value;

  region_delete_value(NULL, v);
}

////////////////////////////////////////////////////////////////////////////////
// builtin utilities

// constructs a tulip definition around a c function
tulip_runtime_toplevel_definition runtime_wrap_builtin(char* fn_name, unsigned int name_length, unsigned int arity) {
  tulip_value names[arity];

  char* builtin_name = alloca(sizeof(char) * (name_length + 8));
  strcpy(builtin_name, "builtin_");
  strcat(builtin_name, fn_name); // [note] strcat may not behave well here

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

runtime_native_defs runtime_create_native_decls(tulip_runtime_module* mod, tulip_runtime_state* state) {
  // [todo] add wrappers to native functions to thread the state->values and mod->module_scope pointers
  runtime_native_defs defs;

  defs.build_tag = LLVMAddFunction(mod->llvm_module, "native_build_tag", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]) { LLVMPointerType(LLVMInt8Type(), 0), LLVMInt64Type(), LLVMPointerType(tulip_value_type, 0)}, 3, false));
  LLVMAddGlobalMapping(state->jit_instance, defs.build_tag, &native_build_tag);

  defs.build

  return defs;
}
