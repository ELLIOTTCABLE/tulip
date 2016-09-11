#pragma once

#include "support/constants.h"

typedef struct tulip_runtime_value tulip_runtime_value;

// decl of typedef in regions.h
typedef unsigned int value_ref;

typedef struct tulip_runtime_tag {
  const char* name;
  unsigned int length;
  value_ref* contents;
} tulip_runtime_tag;

typedef struct tulip_runtime_string {
  unsigned int length;
  const char* value;
} tulip_runtime_string;

typedef struct tulip_runtime_integral {
  long long value;
} tulip_runtime_integral;

typedef struct tulip_runtime_fractional {
  double value;
} tulip_runtime_fractional;

// decl of struct in scopes.h
typedef struct tulip_runtime_scope tulip_runtime_scope;

typedef value_ref (*tulip_runtime_fnptr)(value_ref*);

typedef struct tulip_runtime_closure {
  tulip_runtime_scope* bindings;
  value_ref contents; //fnptr
} tulip_runtime_closure;

typedef struct tulip_runtime_value {
  enum {
    tulip_value_tag,
    tulip_value_string,
    tulip_value_integral,
    tulip_value_fractional,
    tulip_value_closure,
    tulip_value_fnptr
  } type;
  union {
    tulip_runtime_tag        tag;
    tulip_runtime_string     string;
    tulip_runtime_integral   integral;
    tulip_runtime_fractional fractional;
    tulip_runtime_closure    closure;
    tulip_runtime_fnptr      fnptr;
  };
} tulip_runtime_value;
