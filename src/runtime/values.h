#pragma once

typedef struct tulip_runtime_value {
  enum {
    tulip_value_tag,
    tulip_value_string,
    tulip_value_int,
    tulip_value_float,
    tulip_value_closure,
    tulip_value_fnptr
  } tulip_runtime_value_type;
  union {
  };
} tulip_runtime_value;
