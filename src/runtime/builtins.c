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
extern value_ref native_build_tag(tulip_runtime_state* state, const char* tag, unsigned int length, value_ref* contents) {
  tulip_runtime_value v;
  v.type = tulip_value_tag;
  v.tag = (tulip_runtime_tag){tag, length, contents};

  return region_insert_value(state->values, v);
}

extern value_ref native_build_string(tulip_runtime_state* state, const char* value) {
  tulip_runtime_value v = {.type = tulip_value_string, .string = (tulip_runtime_string) {strlen(value), value}};

  return region_insert_value(state->values, v);
}

extern value_ref native_build_integral(tulip_runtime_state* state, long long value) {
  tulip_runtime_value v = {.type = tulip_value_integral, .integral = (tulip_runtime_integral){value}};

  return region_insert_value(state->values, v);
}

extern value_ref native_build_fractional(tulip_runtime_state* state, double value) {
  tulip_runtime_value v = {.type = tulip_value_fractional, .fractional = (tulip_runtime_fractional){value}};

  return region_insert_value(state->values, v);
}

// constructs a closure with an empty scope
extern value_ref native_build_closure(tulip_runtime_state* state, value_ref fn) {
  // [todo] thread scope parent
  // [note] be careful about lexical scoping rules here
  tulip_runtime_scope* s = scope_init(NULL);
  tulip_runtime_value v;
  v.type = tulip_value_closure;
  v.closure = (tulip_runtime_closure){s, fn};

  return region_insert_value(state->values, v);
}

extern value_ref native_build_fnptr(tulip_runtime_state* state, void* fn) {
  tulip_runtime_fnptr fnptr = (tulip_runtime_fnptr) fn;
  tulip_runtime_value v;
  v.type = tulip_value_fnptr;
  v.fnptr = fnptr;

  return region_insert_value(state->values, v);
}

// native value inspection
extern char* native_inspect_type(tulip_runtime_state* state, value_ref v) {
  tulip_runtime_value* val = region_get_value(state->values, v);

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

extern value_ref native_local_scope_lookup(tulip_runtime_scope* scope, const char* name) {
  value_ref ret = scope_lookup(scope, name);

  if (ret) {
    return ret;
  } else {
    assert(false);
    return (value_ref) NULL;
  }
}

extern void native_local_scope_set(tulip_runtime_scope* scope, const char* name, value_ref value) {
  scope_insert(scope, name, value);

  return;
}

// [todo] scope creation

extern void native_pattern_scope_reset(tulip_runtime_state* state, tulip_runtime_scope* scope) {
  tulip_runtime_scope* s = scope_init(scope->parent);
  scope_free(scope, state->values);
  scope = s; // [note] this may not be available in llvm context like i want
}

extern bool native_test_boolean(tulip_runtime_state* state, value_ref v) {
  tulip_runtime_value* val = region_get_value(state->values, v);

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

// [pretty] this looks atrocious and demands to be reworked
runtime_native_defs* runtime_create_native_decls(tulip_runtime_module* mod, tulip_runtime_state* state) {
  runtime_native_defs* defs = malloc(sizeof(runtime_native_defs));

  LLVMValueRef state_ptr = LLVMConstIntToPtr(LLVMConstInt(LLVMInt64Type(), (long long) state, false), LLVMPointerType(LLVMVoidType(), 0));
  // [note] these definitions get collected when their module does, so treating the module pointer as constant is safe
  LLVMValueRef mod_ptr = LLVMConstIntToPtr(LLVMConstInt(LLVMInt64Type(), (long long) mod, false), LLVMPointerType(LLVMVoidType(), 0));

  LLVMValueRef native_fn;
  LLVMValueRef ret;
  LLVMBuilderRef b = LLVMCreateBuilder();

  // build_tag

  native_fn = LLVMAddFunction(mod->llvm_module, "native_build_tag", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMPointerType(LLVMInt8Type(), 0), LLVMInt64Type(), LLVMPointerType(tulip_value_type, 0)}, 4, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->build_tag, &native_build_tag);

  defs->build_tag = LLVMAddFunction(mod->llvm_module, "_native_build_tag", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0), LLVMInt64Type(), LLVMPointerType(tulip_value_type, 0)}, 3, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->build_tag, ""));
  LLVMValueRef name = LLVMGetNextParam(defs->build_tag);
  LLVMValueRef len = LLVMGetNextParam(defs->build_tag);
  LLVMValueRef contents = LLVMGetNextParam(defs->build_tag);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, name, len, contents}, 4, "");
  LLVMBuildRet(b, ret);

  // build_string

  native_fn = LLVMAddFunction(mod->llvm_module, "native_build_string", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMPointerType(LLVMInt8Type(), 0)}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->build_string, &native_build_string);

  defs->build_string = LLVMAddFunction(mod->llvm_module, "_native_build_string", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->build_string, ""));
  LLVMValueRef string = LLVMGetNextParam(defs->build_string);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, string}, 2, "");
  LLVMBuildRet(b, ret);

  // build_integral

  native_fn = LLVMAddFunction(mod->llvm_module, "native_build_integral", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMInt64Type()}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->build_integral, &native_build_string);

  defs->build_integral = LLVMAddFunction(mod->llvm_module, "_native_build_integral", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){LLVMInt64Type()}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->build_integral, ""));
  LLVMValueRef integral = LLVMGetNextParam(defs->build_integral);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, integral}, 2, "");
  LLVMBuildRet(b, ret);

  // build_fractional

  native_fn = LLVMAddFunction(mod->llvm_module, "native_build_fractional", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMDoubleType()}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->build_fractional, &native_build_fractional);

  defs->build_fractional = LLVMAddFunction(mod->llvm_module, "_native_build_fractional", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){LLVMDoubleType()}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->build_fractional, ""));
  LLVMValueRef fractional = LLVMGetNextParam(defs->build_fractional);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, fractional}, 2, "");
  LLVMBuildRet(b, ret);

  // build_closure

  native_fn = LLVMAddFunction(mod->llvm_module, "native_build_closure", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), tulip_value_type}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->build_closure, &native_build_closure);

  defs->build_closure = LLVMAddFunction(mod->llvm_module, "_native_build_closure", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){tulip_value_type}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->build_closure, ""));
  LLVMValueRef target = LLVMGetNextParam(defs->build_closure);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, target}, 2, "");
  LLVMBuildRet(b, ret);

  // build_fnptr

  native_fn = LLVMAddFunction(mod->llvm_module, "native_build_fnptr", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMPointerType(LLVMVoidType(), 0)}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->build_fnptr, &native_build_fnptr);

  defs->build_fnptr = LLVMAddFunction(mod->llvm_module, "_native_build_closure", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){LLVMPointerType(LLVMVoidType(), 0)}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->build_fnptr, ""));
  LLVMValueRef fn = LLVMGetNextParam(defs->build_fnptr);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, fn}, 2, "");
  LLVMBuildRet(b, ret);

  // inspect_type

  // scope_lookup
  // [todo] how should the local scope get passed here?

  defs->local_scope_lookup = LLVMAddFunction(mod->llvm_module, "native_local_scope_lookup", LLVMFunctionType(tulip_value_type, (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMPointerType(LLVMInt8Type(), 0)}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->local_scope_lookup, &native_local_scope_lookup);

  // scope_set

  defs->local_scope_set = LLVMAddFunction(mod->llvm_module, "native_local_scope_set", LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMPointerType(LLVMInt8Type(), 0), tulip_value_type}, 3, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->local_scope_set, &native_local_scope_set);

  // pattern_scope_reset

  native_fn = LLVMAddFunction(mod->llvm_module, "native_pattern_scope_reset", LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), LLVMPointerType(LLVMVoidType(), 0)}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->pattern_scope_reset, &native_pattern_scope_reset);

  defs->pattern_scope_reset = LLVMAddFunction(mod->llvm_module, "_native_pattern_scope_reset", LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef[]){LLVMPointerType(LLVMVoidType(), 0)}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->pattern_scope_reset, ""));
  LLVMValueRef scope = LLVMGetNextParam(defs->pattern_scope_reset);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, scope}, 2, "");
  LLVMBuildRet(b, ret);

  // test_boolean

  native_fn = LLVMAddFunction(mod->llvm_module, "native_test_boolean", LLVMFunctionType(LLVMInt1Type(), (LLVMTypeRef[]){ LLVMPointerType(LLVMVoidType(), 0), tulip_value_type}, 2, false));
  LLVMAddGlobalMapping(state->jit_instance, defs->test_boolean, &native_test_boolean);

  defs->test_boolean = LLVMAddFunction(mod->llvm_module, "_native_test_boolean", LLVMFunctionType(LLVMInt1Type(), (LLVMTypeRef[]){tulip_value_type}, 1, false));
  LLVMPositionBuilderAtEnd(b, LLVMAppendBasicBlock(defs->test_boolean, ""));
  LLVMValueRef subject = LLVMGetNextParam(defs->test_boolean);
  ret = LLVMBuildCall(b, native_fn, (LLVMValueRef[]){state_ptr, subject}, 2, "");
  LLVMBuildRet(b, ret);

  return defs;
}
