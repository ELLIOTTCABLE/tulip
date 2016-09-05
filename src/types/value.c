// value
// this file defines the unified representation for tulip values
// only tags, closures, and literals are distinguished
// this format should be used for all interchange between larger components
// other components should define specialized representations if other distinctions are needed
// [doc] doc/core.org

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "types/value.h"

////////////////////////////////////////////////////////////////////////////////
// builders

tulip_value build_string(char* s) {
  return (tulip_value) {
    .type = TULIP_VALUE_LITERAL,
      .literal = (tulip_literal) {
      .type = TULIP_LITERAL_STRING,
      .string = s
    }
  };
}

tulip_value build_number(double n) {
  return (tulip_value) {
    .type = TULIP_VALUE_LITERAL,
    .literal = (tulip_literal) {
      .type = TULIP_LITERAL_NUMBER,
      .number = n
    }
  };
}

tulip_value build_discrete(long n) {
  return (tulip_value) { .type = TULIP_VALUE_LITERAL
                       , .literal = (tulip_literal) { .type = TULIP_LITERAL_DISCRETE
                                                    , .discrete = n
                                                    }
                       };
}

tulip_value build_tag(char* name, unsigned int length, tulip_value contents[]) {
  tulip_value* c = malloc(sizeof(tulip_value) * length);

  // [note] this memcpy may not be necessary, but i'm leaving it here to hedge against use after free
  if (contents != NULL) memcpy(c, contents, sizeof(tulip_value) * length);

  return (tulip_value) {
    .type = TULIP_VALUE_TAG,
    .tag = (tulip_tag) {
      .name = name,
      .length = length,
      .contents = c
    }
  };
}

void free_tag(tulip_tag* t){}
void free_tulip_value(tulip_value* v){}

////////////////////////////////////////////////////////////////////////////////
// validators

bool validate_tag(tulip_value t) {
  // failed malloc or constructor not used
  if (t.tag.contents == NULL)
    return false;

  // name is either empty or \0
  if (t.tag.name == NULL)
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// pretty printing

char* show_value(tulip_value v) {
  if (v.type == TULIP_VALUE_LITERAL) {
    if (v.literal.type == TULIP_LITERAL_STRING) {
      char* str = malloc(sizeof(char) * (strlen(v.literal.string) + 2));
      sprintf(str, "\"%s\"", v.literal.string);
      return str;
    } else if(v.literal.type == TULIP_LITERAL_NUMBER) {
      char* str = malloc(sizeof(char) * 10);     // [note] length is arbitrary
      snprintf(str, 10, "%g", v.literal.number); // [cont] because this doesn't pad right
      return str;
    }
    return "invalid literal";
  } else if (v.type == TULIP_VALUE_TAG) {
    if (0 < v.tag.length) {
      // todo implement tree printing
      char* str = malloc(sizeof(char) * strlen(v.tag.name) + 5);
      sprintf(str, ".%s ...", v.tag.name);
      return str;
    } else {
      char* str = malloc(sizeof(char) * strlen(v.tag.name) + 1);
      sprintf(str, ".%s", v.tag.name);
      return str;
    }
  } else {
    return "value cannot currently be inspected";
  }
}
