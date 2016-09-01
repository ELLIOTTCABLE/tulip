#pragma once

#include <stdbool.h>

typedef enum tulip_mode {
  none,
  main_file,
  repl,
  // [note] we might want tulip debugging and profiling tools to share the same interface 
  // debug,
  // profile
} tulip_mode;

typedef struct tulip_arguments {
  unsigned int heap_size;
  unsigned int module_limit;
  tulip_mode   mode;
  const char*  main_file;
  const char*  log_file;
  bool         print_help;
  const char*  parse_error;
  unsigned int program_argc;
  const char** program_argv;
} tulip_arguments;

void print_options();
tulip_arguments parse_args(int argc, char* argv[]);
