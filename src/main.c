#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <stdio.h>

#include "runtime/modules.h"
#include "runtime/state.h"
#include "runtime/transforms.h"
#include "types/core.h"
#include "compiler/host.h"
#include "support/options.h"

int main(int argc, char* argv[]) {

  if (argc == 0) {

    print_options();

  } else {

    tulip_arguments args = parse_args(argc, argv);

    if (args.parse_error != NULL) {
      fprintf(stderr, "%s\n", args.parse_error);
      return 0;
    }

    if (args.mode == main_file) {

      // [todo] instantiate compiler

      tulip_runtime_state state = tulip_runtime_start();

      // [todo] poll main module from compiler

      // [todo] invoke main module

      tulip_runtime_stop(state);

      return 0;
    } else if (args.mode == repl) {

      // [todo] repl

    }
  }
}
