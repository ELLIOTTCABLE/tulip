#include <stdio.h>

#include "runtime/modules.h"
#include "runtime/state.h"
#include "runtime/transforms.h"
#include "types/core.h"
#include "compiler/interchange.h"
#include "compiler/host.h"
#include "support/options.h"

int main(int argc, char* argv[]) {

  if (argc <= 1) {

    print_options();

  } else {

    tulip_arguments args = parse_args(argc, argv);

    if (args.parse_error != NULL) {

      fprintf(stderr, "%s\n", args.parse_error);

      return 1;

    }

    tulip_runtime_options o;
    o.heap_size = args.heap_size;
    o.module_limit = args.module_limit;

    // tulip_runtime_state* state = tulip_runtime_start(o);

    if (args.mode == main_file) {
      tulip_compiler_state* compiler = tulip_compiler_start();

      tulip_compiler_compile_file(compiler, args.main_file);

      // [todo] poll main module from compiler
      // [todo] catch compiler errors

      // main_file_output parsed;

      // tulip_runtime_load_main(state, parsed);

      // // [todo] initialize logger

      // tulip_runtime_call_main(state, args.program_argc, args.program_argv);

      // tulip_runtime_stop(state);

      return 0;

    } else if (args.mode == repl) {

      // [todo] repl

      return 0;

    }
  }
}

