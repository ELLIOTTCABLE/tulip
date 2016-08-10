// transforms
// this file defines a set of mutually recursive functions that transform tulip-core tree to LLVM IR

#pragma once

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <llvm-c/Core.h>

#include "runtime/ast.h"
#include "runtime/modules.h"

void tulip_runtime_compile_module(tulip_runtime_module* mod);
