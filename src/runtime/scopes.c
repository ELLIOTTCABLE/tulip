#include "runtime/scopes.h"

#include <string.h>
#include <stdio.h>

tulip_runtime_scope* scope_init(tulip_runtime_scope* parent) {
  tulip_runtime_scope* scope = malloc(sizeof(tulip_runtime_scope));

  scope->length = 0;
  // preallocating 5 elements saves us some time in the most common use cases for scopes
  // scope_insert can avoid realloc until length == 6
  scope->names = malloc(sizeof(char*) * 5);
  scope->bindings = malloc(sizeof(value_ref) * 5);
  scope->parent = parent;

  return scope;
}

void scope_free(tulip_runtime_scope* s, value_region* r) {

  // deallocate tulip values when scopes expire
  // tulip's memory semantics do not permit pass-by-reference, so all values can be freed immediately

  for (unsigned int i = 0; i < s->length; i++) {
    region_delete_value(r, s->bindings[i]);
  }

  free(s->names);
  free(s->bindings);
  free(s);

  return;
}

void scope_insert(tulip_runtime_scope* s, const char* name, value_ref bind) {
  for (unsigned int i = 0; i < s->length; i++) {
    if (strcmp(name, s->names[i]) == 0) {
      s->bindings[i] = bind;
      return;
    }
  }

  if (s->length >= 5) {
    s->names = realloc(s->names, sizeof(char*) * s->length + 1);
    s->bindings = realloc(s->bindings, sizeof(value_ref) * s->length + 1);
  }

  s->names[s->length] = name;
  s->bindings[s->length] = bind;
  s->length += 1;

  return;
}

value_ref scope_lookup(tulip_runtime_scope* s, const char* name) {
  value_ref r;

  for (unsigned int i = 0; i < s->length; i++) {
    if (strcmp(name, s->names[i]) == 0) {
      r = s->bindings[i];
      return r;
    }
  }

  if(s->parent != NULL) {
    return scope_lookup(s->parent, name);
  }

  // [todo] propogate real exception
  // [todo] this error should go to the logger
  fprintf(stderr, "[ERROR-EXCEPTION] lookup for variable %s failed\n", name);

  // [note] [beta] if this return is reached there is a bug either in compiler analysis or in tulip_runtime_transform_name
  return 0;
}
