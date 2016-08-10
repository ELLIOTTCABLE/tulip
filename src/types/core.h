// canonical representation of tulip ast as a tag tree

#include <stdbool.h>

#include "types/value.h"

#pragma once

bool         validate_tree(tulip_value subject);
bool         validate_any(tulip_value subject);

tulip_value  cons(tulip_value array[], int length);
unsigned int cons_length(tulip_value start);
tulip_value* uncons(tulip_value cons);
bool         validate_cons(tulip_value subject);

tulip_value  literal_number(double number);
tulip_value  literal_string(char* string);

tulip_value  name(char* string);

tulip_value  tag(char* name, int length, tulip_value arguments[]);
bool         validate_tag_ast(tulip_value subject);

tulip_value  block(tulip_value contents[], int length);
bool         validate_block(tulip_value subject);

tulip_value  lambda(tulip_value bind, tulip_value body);
bool         validate_lambda(tulip_value subject);

tulip_value  apply(tulip_value call, tulip_value args[], int saturation);
bool         validate_apply(tulip_value subject);

tulip_value  let(tulip_value bind, tulip_value definition);
bool         validate_let(tulip_value subject);

tulip_value  builtin(char* builtin_name, int arity, tulip_value args[], int saturation);
bool         validate_builtin(tulip_value subject);

tulip_value  branch(tulip_value* predicates, tulip_value* conseuqnces, int length);
bool         validate_branch(tulip_value subject);
