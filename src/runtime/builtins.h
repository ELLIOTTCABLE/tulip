#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Core.h>

#include <runtime/ast.h>

tulip_runtime_ast runtime_wrap_builtin(char* name, unsigned int arity);

LLVMModuleRef runtime_create_builtins();
