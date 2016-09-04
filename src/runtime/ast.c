#include "runtime/ast.h"

#include <stdlib.h>
#include <string.h>

#include "types/value.h"
#include "types/core.h"

tulip_runtime_ast_value* convert_tag_to_ast_value(tulip_value t) {
  if (t.type != TULIP_VALUE_TAG) {} // [todo] failure case

  tulip_tag tag = t.tag;

  if (strcmp(tag.name, "block") == 0) {

    unsigned int              length = cons_length(tag.contents[0]);
    tulip_runtime_ast_value** statements = malloc(sizeof(tulip_runtime_ast_value) * length);
    tulip_value*              contents = uncons(tag.contents[0]);

    for(unsigned int i = 0; i < length; i++) {
      statements[i] = convert_tag_to_ast_value(contents[i]);
    }

    tulip_runtime_ast_value* block = malloc(sizeof(tulip_runtime_ast_value));
    block->type = ast_block;
    block->block = (tulip_runtime_ast_block){ length, statements };

    return block;

  } else if (strcmp(tag.name, "apply") == 0) {

    unsigned int              length = cons_length(tag.contents[1]);
    tulip_runtime_ast_value*  call;
    tulip_runtime_ast_value** arguments = malloc(sizeof(tulip_runtime_ast_value) * length);
    tulip_value*              contents = uncons(tag.contents[1]);

    call = convert_tag_to_ast_value(tag.contents[0]);

    for (unsigned int i = 0; i < length; i++) {
      arguments[i] = convert_tag_to_ast_value(contents[i]);
    }

    tulip_runtime_ast_value* apply = malloc(sizeof(tulip_runtime_ast_value));

    apply->type = ast_apply;
    apply->apply = (tulip_runtime_ast_apply){ call, .saturation = length, arguments };

    return apply;

  } else if (strcmp(tag.name, "lambda") == 0) {

    tulip_runtime_ast_value* bind_v     = convert_tag_to_ast_value(tag.contents[0]);
    tulip_runtime_ast_value* expression = convert_tag_to_ast_value(tag.contents[1]);

    if (bind_v->type != ast_literal || bind_v->literal.type != ast_literal_string) {} // [todo] failure case

    tulip_runtime_ast_name bind = (tulip_runtime_ast_name){NULL, 0, bind_v->literal.string};

    tulip_runtime_ast_value* lambda = malloc(sizeof(tulip_runtime_ast_value));
    lambda->type = ast_lambda;
    lambda->lambda = (tulip_runtime_ast_lambda){ bind, expression };

    return lambda;

  } else if (strcmp(tag.name, "let") == 0) {

    tulip_runtime_ast_value* bind_v     = convert_tag_to_ast_value(tag.contents[0]);
    tulip_runtime_ast_value* definition = convert_tag_to_ast_value(tag.contents[1]);

    if (bind_v->type != ast_literal || bind_v->literal.type != ast_literal_string) {} // [todo] failure case

    tulip_runtime_ast_name bind = (tulip_runtime_ast_name){NULL, 0, bind_v->literal.string};

    tulip_runtime_ast_value* lambda = malloc(sizeof(tulip_runtime_ast_value));
    lambda->type = ast_let;
    lambda->let  = (tulip_runtime_ast_let){ bind, definition };

    return lambda;

  } else if (strcmp(tag.name, "branch") == 0) {

    unsigned int              length = cons_length(tag.contents[0]);
    tulip_runtime_ast_value** predicates = malloc(sizeof(tulip_runtime_ast_value) * length);
    tulip_runtime_ast_value** consequences = malloc(sizeof(tulip_runtime_ast_value) * length);
    tulip_value*              p_contents = uncons(tag.contents[0]);
    tulip_value*              c_contents = uncons(tag.contents[1]);

    for (unsigned int i = 0; i < length; i++) {
      predicates[i] = convert_tag_to_ast_value(p_contents[i]);
      consequences[i] = convert_tag_to_ast_value(c_contents[i]);
    }

    tulip_runtime_ast_value* branch = malloc(sizeof(tulip_runtime_ast_value));
    branch->type = ast_branch;
    branch->branch = (tulip_runtime_ast_branch){ length, predicates, consequences };

    return branch;

  } else if (strcmp(tag.name, "builtin") == 0) {

    unsigned int              length = cons_length(tag.contents[2]);
    tulip_runtime_ast_value*  call_v;
    tulip_runtime_ast_name    call;
    tulip_runtime_ast_value*  arity_v;
    unsigned int              arity;
    tulip_runtime_ast_value** arguments = malloc(sizeof(tulip_runtime_ast_value) * length);
    tulip_value*              contents = uncons(tag.contents[2]);

    call_v = convert_tag_to_ast_value(tag.contents[0]);
    arity_v = convert_tag_to_ast_value(tag.contents[1]);

    if (call_v->type != ast_name || arity_v->type != ast_literal || arity_v->literal.type != ast_literal_number) {} // [todo] failure case

    call = call_v->name;
    arity = arity_v->literal.number;

    for (unsigned int i = 0; i < length; i++) {
      arguments[i] = convert_tag_to_ast_value(contents[i]);
    }

    tulip_runtime_ast_value* builtin = malloc(sizeof(tulip_runtime_ast_value));
    builtin->type = ast_builtin;
    builtin->builtin = (tulip_runtime_ast_builtin){ call, arity, .saturation = length, arguments };

    return builtin;

  } else if (strcmp(tag.name, "literal") == 0) {

    tulip_runtime_ast_value* literal = malloc(sizeof(tulip_runtime_ast_value));
    literal->type = ast_literal;
    if (tag.contents[0].literal.type == TULIP_LITERAL_STRING)
      literal->literal = (tulip_runtime_ast_literal){ .type = ast_literal_string, .string = tag.contents[0].literal.string };
    if (tag.contents[0].literal.type == TULIP_LITERAL_NUMBER)
      literal->literal = (tulip_runtime_ast_literal){ .type = ast_literal_number, .number = tag.contents[0].literal.number };

    return literal;

  } else if (strcmp(tag.name, "name") == 0) {
    unsigned int length = cons_length(tag.contents[0]);
    tulip_runtime_ast_value* name = malloc(sizeof(tulip_runtime_ast_value));

    name->type = ast_name;
    if (length > 0) {
      name->name = (tulip_runtime_ast_name){NULL, 0, tag.contents[1].literal.string};
    } else {
      tulip_value* contents = uncons(tag.contents[9]);
      char* strs[length];
      tulip_value v;

      for (unsigned int i = 0; i < length; i++) {
        v = contents[i];
        if (v.type == TULIP_VALUE_LITERAL && v.literal.type == TULIP_LITERAL_STRING)
          strs[i] = v.literal.string;
      }

      name->name = (tulip_runtime_ast_name){strs, length, tag.contents[1].literal.string};
    }

    return name;

  } else {
    return NULL;
  }
}

void destroy_ast_value(tulip_runtime_ast_value* v){

  switch (v->type) {

  case ast_name:
    // [note] does a char* need to be freed?
    // free(v->name.name);
    // for (unsigned int i = 0; i < v->name.modulePathLen; i++) { free(v->name.modulePath[i]); }
    free(v->name.modulePath);
    break;

  case ast_literal:
    // if (v->literal.type == ast_literal_string) free(v->literal.string);
    break;

  case ast_block:
    for (unsigned int i = 0; i < v->block.length; i++) {
      destroy_ast_value(v->block.statements[i]);
    }
    free(v->block.statements);
    break;

  case ast_let:
    destroy_ast_value(v->let.definition);
    break;

  case ast_apply:
    destroy_ast_value(v->apply.call);
    for (unsigned int i = 0; i < v->apply.saturation; i++) {
      destroy_ast_value(v->apply.arguments[i]);
    }
    free(v->apply.arguments);
    break;

  case ast_lambda:
    destroy_ast_value(v->lambda.expression);
    break;

  case ast_branch:
    for (unsigned int i = 0; i < v->branch.length; i++) {
      destroy_ast_value(v->branch.predicates[i]);
      destroy_ast_value(v->branch.consequences[i]);
    }
    free(v->branch.predicates);
    free(v->branch.consequences);
    break;

  case ast_builtin:
    for (unsigned int i = 0; i < v->builtin.saturation; i++) {
      destroy_ast_value(v->builtin.arguments[i]);
    }
    free(v->builtin.arguments);
    break;

  case ast_tag:
    for (unsigned int i = 0; i < v->tag.contents_length; i++) {
      destroy_ast_value(v->tag.contents[i]);
    }
    free(v->tag.contents);
    break;
  }

  free(v);
}

char* show_ast(tulip_runtime_ast_value* ast) {}

char* render_qualified_name(tulip_runtime_ast_name name, tulip_runtime_module* context) {
  char* rendered_path;

  if (name.modulePathLen == 0) {

    // qualify to local module

    for (unsigned int i = 0; i < context->path_len; i++) {
      rendered_path = strcat(rendered_path, context->path[i]);
      rendered_path = strcat(rendered_path, "_");
    }

    rendered_path = strcat(rendered_path, context->name);
    rendered_path = strcat(rendered_path, "_");

  } else {

    // qualify to foreign module

    for (unsigned int i = 0; i < name.modulePathLen; i++) {
      rendered_path = strcat(rendered_path, name.modulePath[i]);
      rendered_path = strcat(rendered_path, "_");
    }
  }

  return strcat(rendered_path, name.name);
}
