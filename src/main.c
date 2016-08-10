#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <stdio.h>

#include "runtime/modules.h"
#include "runtime/state.h"
#include "runtime/transforms.h"
#include "types/core.h"
#include "compiler/host.h"

int main(int argc, char* argv[]) {
  tulip_runtime_module testmod = (tulip_runtime_module) {
    .version         = null_module_version,
    .status          = TULIP_MODULE_UNCOMPILED,
    .definitions     = (tulip_runtime_toplevel_definition[]){
      (tulip_runtime_toplevel_definition) {
        .name        = "testblock",
        .definition  = block( (tulip_value[]){
            apply( builtin("print", 1, (tulip_value[]){}, 0), (tulip_value[]){ literal_string("test") }, 1 )
        }, 1)
      }
    },
    .num_definitions = 1,
    .llvm_module     = NULL
  };

  printf("CORE AST:\n\t");
  printf("%s\n", show_value(testmod.definitions[0].definition));

  /* tulip_runtime_ast_value* rt_def = convert_tag_tree_to_ast(testmod.definitions[0].definition); */

  /* printf("RT AST:\n\t"); */
  /* printf("%s\n", show_ast(rt_def)); */

  /* printf("LLVM IR:\n\t"); */
  tulip_runtime_state state = tulip_runtime_start();

  tulip_runtime_stop(state);

  return 0;
}
