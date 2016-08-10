// unified representation for tulip values

#pragma once

typedef struct tulip_literal {
  enum literal_tag { TULIP_LITERAL_STRING
                   , TULIP_LITERAL_NUMBER
                   , TULIP_LITERAL_DISCRETE
                   } type;
  union {
    char* string;
    double number;
    long discrete;
  };
} tulip_literal;

// forward decl
typedef struct tulip_value tulip_value;

typedef struct tulip_tag {
  char* name;
  unsigned int length;
  tulip_value* contents;
} tulip_tag;

typedef struct tulip_value {
  enum value_tag { TULIP_VALUE_LITERAL
                 , TULIP_VALUE_CLOSURE
                 , TULIP_VALUE_TAG
                 } type;
  union {
    tulip_literal literal;
    void* closure;
    tulip_tag tag;
  };
} tulip_value;
#define is_literal(v, t) ((v).type == TULIP_VALUE_LITERAL && (v).literal.type == t)

tulip_value build_string(char* s);
#define is_string(v) is_literal(v, TULIP_LITERAL_STRING)
#define unwrap_string(v) (is_string(v) ? (v).literal.string : NULL)

tulip_value build_number(double n);
#define is_number(v) (is_literal(v, TULIP_LITERAL_NUMBER))
#define unwrap_number(v) (is_number(v) ? (v).literal.number : NULL)

tulip_value build_discrete(long n);
#define is_discrete(v) (is_literal(v, TULIP_LITERAL_DISCRETE))
#define unwrap_discrete(v) (is_discrete(v) ? (v).literal.discrete : NULL)

tulip_value build_tag(char* name, unsigned int length, tulip_value contents[]);
char*       show_value(tulip_value v);

tulip_tag*  append_tag(tulip_tag* t, tulip_value v);
