#include <stdlib.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>

#include "compiler/host.h"

static void stackDump (lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        printf("`%s'", lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        printf(lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        printf("%g", lua_tonumber(L, i));
        break;

      default:  /* other values */
        printf("%s", lua_typename(L, t));
        break;

    }
    printf("  ");  /* put a separator */
  }
  printf("\n");  /* end the listing */
}

char_reader_state* char_reader_setup(char* input, int length) {
  char_reader_state* state = malloc(sizeof(char_reader_state));
  state->index  = 0;
  state->input  = input;
  state->length = length;
  return state;
}

int Lchar_reader_setup(lua_State* s) {
  char* input = lua_tostring(s, 1);
  int length = lua_tointeger(s, 2);

  lua_pushlightuserdata(s, char_reader_setup(input, length));

  return 1;
}

void char_reader_teardown(char_reader_state* state) {
  free(state);
}

int Lchar_reader_teardown(lua_State* s) {
  char_reader_state* state = lua_topointer(s, 1);

  char_reader_teardown(state);

  return 0;
}

char char_reader_next(char_reader_state* state) {
  if(state->index < state->length) {
    char v = state->input[state->index];
    state->index = state->index + 1;
    return v;
  } else { return (char) NULL; }
}

int Lchar_reader_next(lua_State* s) {
  char_reader_state* state = lua_topointer(s, 1);

  char out = char_reader_next(state);

  if (!out) return 0;

  // [jneen] this is sad.
  char b[2] = { out, '\0' };

  lua_pushstring(s, b);

  return 1;
}

tulip_value compiler_create_tag(char* name, int length, tulip_value* values) {
  return build_tag(name, length, values);
}

int Lcompiler_create_tag(lua_State* s) {
  char* name = lua_tostring(s, 1);
  int length = lua_tointeger(s, 2);

  printf("Lcompiler_create_tag: %s/%d\n", name, length);

  tulip_value *contents = NULL;

  if (length > 0) {
    // [pretty] this is an awful workaround and can be avoided
    contents = malloc(length * sizeof(tulip_value));

    for (int i = 0; i < length; i++) {
      tulip_value* elem = (tulip_value *) lua_topointer(s, i+3);
      if (elem == NULL) stackDump(s);

      memcpy(&contents[i], elem, sizeof(tulip_value));
    }
  }

  tulip_value out = build_tag(name, length, contents);

  // break

  tulip_value *value = malloc(sizeof(tulip_value));
  memcpy(value, &out, sizeof(tulip_value));

  lua_pushlightuserdata(s, value);
  return 1;
}

tulip_value* compiler_tag_get(tulip_value tag, unsigned int index) {
  if (tag.type != TULIP_VALUE_TAG) return NULL;

  if (index < tag.tag.length) {
    return &tag.tag.contents[index];
  } else {
    return NULL;
  }
}

int Lcompiler_tag_get(lua_State* s) {
  tulip_value* tag = (tulip_value*) lua_topointer(s, 1);
  unsigned int index = lua_tointeger(s, 2);

  tulip_value* value = compiler_tag_get(*tag, index);
  if (value == NULL) return 0;

  lua_pushlightuserdata(s, value);
  return 1;
}

bool compiler_matches_tag(tulip_value tag, char* name, int arity) {
  return (tag.type == TULIP_VALUE_TAG &&
          strcmp(tag.tag.name, name) == 0 &&
          tag.tag.length == (unsigned int) arity);
}

int Lcompiler_matches_tag(lua_State* s) {
  tulip_value* tag = (tulip_value*) lua_topointer(s, 1);
  char* name = lua_tostring(s, 2);
  int arity = lua_tointeger(s, 3);

  lua_pushboolean(s, compiler_matches_tag(*tag, name, arity));
  return 1;
}

int Lcompiler_tag_name(lua_State* s) {
  tulip_value* tag = (tulip_value*) lua_topointer(s, 1);

  if (tag->type != TULIP_VALUE_TAG) return 0;

  lua_pushstring(s, tag->tag.name);
  return 1;
}

int Lcompiler_tag_arity(lua_State* s) {
  tulip_value* tag = (tulip_value*) lua_topointer(s, 1);

  if (tag->type != TULIP_VALUE_TAG) return 0;

  lua_pushinteger(s, tag->tag.length);
  return 1;
}

int Lcompiler_tag_values(lua_State* s) {
  tulip_value* tag = (tulip_value*) lua_topointer(s, 1);

  if (tag->type != TULIP_VALUE_TAG) return 0;

  for (int i = 0; i < tag->tag.length; i++) {
    lua_pushlightuserdata(s, &tag->tag.contents[i]);
  }

  return tag->tag.length;
}

int Lcompiler_is_tag(lua_State* s) {
  // TODO
}

char* compiler_inspect_value(tulip_value value) {
  // [recheck] does printing values need to be specialized for the compiler?
  return show_value(value);
}

int Lcompiler_inspect_value(lua_State* s) {
  tulip_value* value = (tulip_value*) lua_topointer(s, 1);

  lua_pushstring(s, compiler_inspect_value(*value));
  return 1;
}

void push_tulip_value(lua_State *s, tulip_value value) {
  // [jneen] uh, leaks
  tulip_value* value_p = malloc(sizeof(tulip_value));
  memcpy(value_p, &value, sizeof(tulip_value));

  lua_pushlightuserdata(s, value_p);
}

int Lcompiler_build_string(lua_State *s) {
  char *string = lua_tostring(s, 1);
  tulip_value value = build_string(string);

  push_tulip_value(s, value);

  return 1;
}

int Lcompiler_unwrap_string(lua_State *s) {
  int *foo;
  tulip_value* value = (tulip_value*) lua_topointer(s, 1);

  if (!is_string(*value)) {
    *foo = 0;
    return 0;
  }

  lua_pushstring(s, unwrap_string(*value));

  return 1;
}

int Lcompiler_build_discrete(lua_State *s) {
  long number = lua_tointeger(s, 1);
  tulip_value value = build_discrete(number);

  push_tulip_value(s, value);

  return 1;
}

int Lcompiler_unwrap_discrete(lua_State *s) {
  int *foo;
  tulip_value* value = (tulip_value*) lua_topointer(s, 1);

  if (!is_discrete(*value)) {
    *foo = 0; // GDB
    return 0;
  }

  lua_pushinteger(s, unwrap_discrete(*value));

  return 1;
}

tulip_compiler_state* tulip_compiler_start() {
  tulip_compiler_state* state = malloc(sizeof(tulip_compiler_state));

  lua_State *lua = luaL_newstate();
  state->lua_state = lua;

  luaL_openlibs(lua);

  // wrap and inject the utility functions defined above
  lua_pushcfunction(lua, Lchar_reader_setup);
  lua_setglobal(lua, "__compiler_reader_setup");
  lua_pushcfunction(lua, Lchar_reader_teardown);
  lua_setglobal(lua, "__compiler_reader_teardown");
  lua_pushcfunction(lua, Lchar_reader_next);
  lua_setglobal(lua, "__compiler_reader_next");

  lua_pushcfunction(lua, Lcompiler_create_tag);
  lua_setglobal(lua, "__compiler_create_tag");
  lua_pushcfunction(lua, Lcompiler_tag_get);
  lua_setglobal(lua, "__compiler_tag_get");
  lua_pushcfunction(lua, Lcompiler_matches_tag);
  lua_setglobal(lua, "__compiler_matches_tag");
  lua_pushcfunction(lua, Lcompiler_inspect_value);
  lua_setglobal(lua, "__compiler_inspect_value");

  lua_pushcfunction(lua, Lcompiler_build_string);
  lua_setglobal(lua, "__compiler_build_string");

  lua_pushcfunction(lua, Lcompiler_unwrap_string);
  lua_setglobal(lua, "__compiler_unwrap_string");

  lua_pushcfunction(lua, Lcompiler_build_discrete);
  lua_setglobal(lua, "__compiler_build_discrete");

  lua_pushcfunction(lua, Lcompiler_unwrap_discrete);
  lua_setglobal(lua, "__compiler_unwrap_discrete");

  lua_pushcfunction(lua, Lcompiler_tag_name);
  lua_setglobal(lua, "__compiler_tag_name");

  lua_pushcfunction(lua, Lcompiler_tag_arity);
  lua_setglobal(lua, "__compiler_tag_arity");

  lua_pushcfunction(lua, Lcompiler_tag_values);
  lua_setglobal(lua, "__compiler_tag_values");

  // [todo] better loading of lua sources
  if (luaL_dofile(lua, "./lua/export.lua")) {
    printf("lua loadtime error: %s\n", lua_tostring(lua, -1));
  }

  return state;
}

main_file_output tulip_compiler_compile_file(tulip_compiler_state* state, char* file_name) {
  lua_State *lua = state->lua_state;

  lua_getglobal(lua, "compile_file");

  lua_pushstring(lua, file_name);

  if (lua_pcall(lua, 1, 0, 0) != 0) {
    printf("lua runtime error: %s\n", lua_tostring(lua, -1));
  }

  main_file_output o;
  return o;
}


void tulip_compiler_stop(tulip_compiler_state* state) {
  lua_close(state->lua_state);
}
