#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "runtime/init.h"
#include "compiler/host.h"

int main(int argc, char* argv[]) {
  tulip_compiler_state* state = tulip_compiler_start();

  tulip_compiler_stop(state);

  return 0;
}
