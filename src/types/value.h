// value
// this file defines the unified representation for tulip values
// only tags, closures, and literals are distinguished
// this format should be used for all interchange between larger components
// other components should define specialized representations if other distinctions are needed
// [doc] doc/core.org

#pragma once

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// literals

typedef enum {
  TULIP_LITERAL_STRING,
  TULIP_LITERAL_NUMBER,
  TULIP_LITERAL_DISCRETE
} literal_type;

typedef struct {
  literal_type type;
  union {
    char* string;
    double number;
    long discrete;
  };
} tulip_literal;

////////////////////////////////////////////////////////////////////////////////
// tags and value distinctions

// forward decl of value for use in tag
typedef struct tulip_value tulip_value;

typedef struct {
  char* name;
  unsigned int length;
  tulip_value* contents;
} tulip_tag;

typedef enum {
  TULIP_VALUE_LITERAL,
  TULIP_VALUE_CLOSURE,
  TULIP_VALUE_TAG
} value_type;

typedef struct tulip_value { // [note] double name required by forward decl
  value_type type;
  union {
    tulip_literal literal;
    void* closure; // [todo] define a type for closures
    tulip_tag tag;
  };
} tulip_value;
#define is_literal(v, t) ((v).type == TULIP_VALUE_LITERAL && (v).literal.type == t)


////////////////////////////////////////////////////////////////////////////////
// builders

tulip_value build_string(char* s);
#define is_string(v) is_literal(v, TULIP_LITERAL_STRING)
#define unwrap_string(v) (is_string(v) ? (v).literal.string : NULL)

tulip_value build_discrete(long n);
tulip_value build_number(double n);
#define is_number(v) (is_literal(v, TULIP_LITERAL_NUMBER))
#define unwrap_number(v) (is_number(v) ? (v).literal.number : NULL)

tulip_value build_discrete(long n);
#define is_discrete(v) (is_literal(v, TULIP_LITERAL_DISCRETE))
#define unwrap_discrete(v) (is_discrete(v) ? (v).literal.discrete : NULL)

tulip_value build_tag(char* name, unsigned int length, tulip_value contents[]);
char*       show_value(tulip_value v);

tulip_value build_tag(char* name, unsigned int length, tulip_value contents[]);
tulip_tag*  append_tag(tulip_tag* t, tulip_value v);

////////////////////////////////////////////////////////////////////////////////
// validators

bool validate_tag(tulip_value t);

////////////////////////////////////////////////////////////////////////////////
// pretty printing

char* show_value(tulip_value v);

