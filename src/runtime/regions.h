#pragma once

#include <stdlib.h>

#include "runtime/modules.h"
#include "runtime/values.h"

////////////////////////////////////////////////////////////////////////////////
// value region

typedef unsigned int value_ref;

typedef struct value_metadata {
  bool is_free;
  tulip_runtime_value value;
} value_metadata;

typedef struct value_region {
  unsigned int num_values;
  unsigned int num_used;
  // allocations are attempted at the cursor first
  // in the future we should actually track the heap partitions introduced by the gc
  unsigned int cursor;

  size_t mem_length;
  value_metadata* region;
} value_region;

value_region*        init_value_region(unsigned int num_values);
void                 free_value_region(value_region* r);

void                 check_value_region(value_region* r);
value_ref            region_insert_value(value_region* r, tulip_runtime_value v);
void                 region_delete_value(value_region* r, value_ref v);
tulip_runtime_value* region_get_value(value_region* r, value_ref v);

////////////////////////////////////////////////////////////////////////////////
// module region

typedef unsigned int module_ref;

typedef struct module_metadata {
  bool is_free;
  tulip_runtime_module mod;
} module_metadata;

typedef struct module_region {
  unsigned int num_modules;
  unsigned int num_used;
  unsigned int cursor;

  size_t mem_length;
  module_metadata* region;
} module_region;

module_region*        init_module_region(unsigned int num_modules);
void                  free_module_region(module_region* r);

module_ref            region_insert_module(module_region* r, tulip_runtime_module m);
void                  region_delete_module(module_region* r, module_ref m);

tulip_runtime_module* region_query_module_by_id(module_region* r, module_ref m);
tulip_runtime_module* region_query_module_by_name(const char* name);
tulip_runtime_module* region_query_module_by_version(const char* name, tulip_runtime_module_version ver);
