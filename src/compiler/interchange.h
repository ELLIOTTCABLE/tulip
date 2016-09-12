#pragma once

#include "types/value.h"

typedef struct main_file_input {
  char* file;
  char* file_dir;
  char* working_dir;
} main_file_input;

typedef struct module_output {
  char* name;
  unsigned int path_len;
  char** path;
  unsigned int num_definitions;
  char** names;
  tulip_value* definitions;
} module_output;

typedef enum {
  compile_succeeded,
  compile_failed
  // [note] this is intended to be replaced with a better failure mechanism
} compilation_status;

typedef struct main_file_output {
  module_output main_module;
  unsigned int num_dependencies;
  module_output* dependencies;
  char* error;
} main_file_output;
