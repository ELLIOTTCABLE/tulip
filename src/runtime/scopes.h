#pragma once

#include "runtime/regions.h"

typedef struct tulip_runtime_scope {
  unsigned int length;
  const char** names;
  value_ref*   bindings;
  struct tulip_runtime_scope* parent;
} tulip_runtime_scope;

tulip_runtime_scope* scope_init(tulip_runtime_scope* parent);
void scope_free(tulip_runtime_scope* s, value_region* r);
void scope_insert(tulip_runtime_scope* s, const char* name, value_ref bind);
value_ref scope_lookup(tulip_runtime_scope* s, const char* name);
