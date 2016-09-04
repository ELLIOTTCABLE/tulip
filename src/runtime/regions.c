#include "runtime/regions.h"

////////////////////////////////////////////////////////////////////////////////
// value region and collector

value_region* init_value_region(unsigned int num_values) {
  value_region* region_struct = malloc(sizeof(value_region));
  region_struct->num_values = num_values;
  region_struct->num_used = 0;
  region_struct->cursor = 0;
  region_struct->region = malloc(sizeof(value_metadata) * num_values);

  for(unsigned int i = 0; i < num_values; i++) {
    size_t offset = i * sizeof(value_metadata);
    value_metadata* m = region_struct->region + offset;
    m->is_free = true;
  }

  return region_struct;
}

// naive gc on tulip heap
// [todo]
// will be left unimplemented until necessary
void collect_value_region(value_region* r){ return; }

// verifies space is available to be allocated
// if not, run the gc
void check_value_region(value_region* r) {
  if (r->num_used >= r->num_values) collect_value_region(r);

  while (!r->region[r->cursor].is_free) {
    if (r->cursor < r->num_values - 1)
      r->cursor += 1;
    else if (r->cursor == r->num_values - 1)
      r->cursor  = 0;
  }

  return;
}

value_ref region_insert_value(value_region* r, tulip_runtime_value v){
  check_value_region(r);

  r->region[r->cursor].is_free = false;
  r->region[r->cursor].value = v;

  r->num_used += 1;
  return r->cursor;
}

void region_delete_value(value_region* r, value_ref v) {
  r->region[v].is_free = true;
}

tulip_runtime_value* region_get_value(value_region* r, value_ref v) {
  if (!r->region[v].is_free) {
    return &r->region[v].value;
  } else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
// module region and queries

module_region* init_module_region(unsigned int num_modules) {
  module_region* region_struct = malloc(sizeof(module_region));
  region_struct->num_modules = num_modules;
  region_struct->num_used = 0;
  region_struct->cursor = 0;
  region_struct->region = malloc(sizeof(module_metadata) * num_modules);
}

module_ref region_insert_module(module_region* r, tulip_runtime_module m) {
  // if(r->num_used >= r->num_modules) [todo] error case when module limit is exceeded

  r->cursor += 1;

  r->region[r->cursor].is_free = false;
  r->region[r->cursor].mod = m;

  r->num_used += 1;
  return r->cursor;
}

void region_delete_module(module_region* r, module_ref m) {
  r->region[r->cursor].is_free = true;
}

tulip_runtime_module* region_query_module_by_name(const char* name) {
}

tulip_runtime_module* region_query_module_by_version(const char* name, tulip_runtime_module_version ver) {
}
