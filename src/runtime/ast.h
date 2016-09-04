// specialized ast representation for tulip-core -> llvm ir transforms

#pragma once

#include "types/core.h"
#include "types/value.h"
#include "runtime/modules.h"

typedef enum {
  ast_block,
  ast_lambda,
  ast_apply,
  ast_let,
  ast_branch,
  ast_builtin,
  ast_tag,
  ast_name,
  ast_literal,
} tulip_runtime_ast_type;

typedef struct tulip_runtime_ast_value tulip_runtime_ast_value;

typedef struct tulip_runtime_ast_name {
  char** modulePath;
  unsigned int modulePathLen;
  char*  name;
} tulip_runtime_ast_name;

typedef struct tulip_runtime_ast_literal {
  enum {
    ast_literal_string,
    ast_literal_int,
    ast_literal_float
  } type;
  union {
    long long integral;
    double fractional;
    char* string;
  };
} tulip_runtime_ast_literal;

typedef struct tulip_runtime_ast_block {
  unsigned int              length;
  tulip_runtime_ast_value** statements;
} tulip_runtime_ast_block;

typedef struct tulip_runtime_ast_let {
  tulip_runtime_ast_name   bind;
  tulip_runtime_ast_value* definition;
} tulip_runtime_ast_let;

typedef struct tulip_runtime_ast_apply {
  tulip_runtime_ast_value*  call;
  unsigned int              saturation;
  tulip_runtime_ast_value** arguments;
} tulip_runtime_ast_apply;

typedef struct tulip_runtime_ast_lambda {
  tulip_runtime_ast_name   bind;
  tulip_runtime_ast_value* expression;
} tulip_runtime_ast_lambda;

typedef struct tulip_runtime_ast_branch {
  unsigned int              length;
  tulip_runtime_ast_value** predicates;
  tulip_runtime_ast_value** consequences;
} tulip_runtime_ast_branch;

typedef struct tulip_runtime_ast_builtin {
  tulip_runtime_ast_name    call;
  unsigned int              arity;
  unsigned int              saturation;
  tulip_runtime_ast_value** arguments;
} tulip_runtime_ast_builtin;

typedef struct tulip_runtime_ast_tag {
  char*                     tag_name;
  unsigned int              contents_length;
  tulip_runtime_ast_value** contents;
} tulip_runtime_ast_tag;

typedef struct tulip_runtime_ast_value {
  tulip_runtime_ast_type type;
  union {
    tulip_runtime_ast_block   block;
    tulip_runtime_ast_apply   apply;
    tulip_runtime_ast_lambda  lambda;
    tulip_runtime_ast_let     let;
    tulip_runtime_ast_branch  branch;
    tulip_runtime_ast_builtin builtin;
    tulip_runtime_ast_name    name;
    tulip_runtime_ast_literal literal;
    tulip_runtime_ast_tag     tag;
  };
} tulip_runtime_ast_value;

tulip_runtime_ast_value* convert_tag_to_ast_value(tulip_value t);
void                     destroy_art_value(tulip_runtime_ast_value* v);
char*                    show_ast(tulip_runtime_ast_value* ast);
char*                    render_qualified_name(tulip_runtime_ast_name name, tulip_runtime_module* context);
