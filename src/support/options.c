#include "support/options.h"

#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void print_options() {
  printf("    )`(           )`(\n");
  printf("   (   )  tulip  (   )\n");
  printf("     |/            |/\n");
  printf("\n");
  printf("      usage:\n");
  printf("  tulip [options] <action> -- [tulip-options]\n");
  printf("\n");
  printf("      actions:\n");
  printf("  run [file] [fn]     : call the function `fn` in tulip module `file`\n");
  printf("                        fn defaults to `main`\n");
  printf("  repl                : runs the tulip repl\n");
  printf("\n");
  printf("      options:\n");
  printf("  -m | --heap-size    : runtime heap size, in number of tulip values\n");
  printf("                        defaults to %d\n", 2 << 16);
  printf("  -s | --module-limit : number of modules allowed at one time\n");
  printf("                        defaults to 128\n");
  printf("  -l | --log-file     : file to use for runtime logging\n");
  printf("  -h | --help         : print detailed help for a command");
}

// [todo] help for individual modes

unsigned int parse_uint(const char* t) {
  // [todo] int parser
}

tulip_arguments parse_args(int argc, char* argv[]) {
  tulip_arguments args;

  // default values

  args.main_file = NULL;
  args.log_file = NULL;
  args.module_limit = 128;
  args.heap_size = 2 << 16;
  args.print_help = false;
  args.parse_error = NULL;
  args.program_argc = 0;
  args.program_argv = NULL;
  args.mode = none;

  int i = 1;
  while (i < argc) {

    if (argv[i][0] == '-' && argv[i][1] == '-') {

      // long options

      if (strncmp(argv[i], "--heap-size=", 12) == 0) {

        args.heap_size = parse_uint(&argv[i][11]);

      } else if (strncmp(argv[i], "--module-limit=", 15) == 0) {

        args.module_limit = parse_uint(&argv[i][14]);

      } else if (strncmp(argv[i], "--log-file=", 11) == 0) {

        args.log_file = &argv[i][10];

      } else if (strcmp(argv[i], "--help") == 0) {

        args.print_help = true;

      }

    } else if (argv[i][0] == '-') {

      // short options

      switch (argv[i][1]) {

      case 'm':
        args.heap_size = parse_uint(argv[i+1]);
        i += 2;
        break;

      case 's':
        args.module_limit = parse_uint(argv[i+1]);
        i += 2;
        break;

      case 'l':
        args.log_file = argv[i+1];
        i += 2;
        break;

      case 'h':
        args.print_help = true;
        i += 1;
        break;

      case '-':
        args.program_argc = argc - (i + 1);
        args.program_argv = malloc(sizeof(char*) * args.program_argc);
        memcpy(args.program_argv, argv[i+1], sizeof(char*) * args.program_argc);
        i = argc;
        break;

      default:
        break;
      }

    } else {

      if (strcmp(argv[i], "run") == 0) {

        args.mode = main_file;
        args.main_file = argv[i+1];

        i += 2;

      } else if (strcmp(argv[i], "repl") == 0) {

        args.mode = repl;

        i += 1;

      } else {
        break;
      }

    }
  }

  // validate arguments

  // [todo] only tag these errors if categorized logging is enabled

  if (args.mode == none) {
    args.parse_error = "[ERROR-MISCONFIGURED] Please specify an action";
    return args;
  }

  if(access(args.main_file, F_OK)) {
    args.parse_error = strcat("[ERROR-MISCONFIGURED] Could not find main module at ", args.main_file);
    return args;
  }

  if(open(args.log_file, O_CREAT | O_APPEND) == -1) {
    args.parse_error = strcat("[ERROR-MISCONFIGURED] Could not open logfile ", args.log_file);
    return args;
  }

  if(args.heap_size == 0) {
    args.parse_error = "[ERROR-MISCONFIGURED] Invalid heap size value";
    return args;
  }

  if(args.module_limit == 0) {
    args.parse_error = "[ERROR-MISCONFIGURED] Invalid module limit value";
    return args;
  }

  return args;
}
