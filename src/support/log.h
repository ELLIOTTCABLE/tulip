#pragma once

#include <errno.h>
#include <stdio.h>

typedef struct {
  // [note] logging formats to be decided later
} tulip_logger_options;

tulip_logger_options tulip_logger_default_options = {};

typedef struct {
  enum {
    tulip_logger_uninitialized,
    tulip_logger_running,
    tulip_logger_failure,
    tulip_logger_closed
  } tulip_logger_status;

  int tulip_logger_error;

  FILE* tulip_logger_file;

  tulip_logger_options options;

} tulip_logger_state;

tulip_logger_state tulip_logging_setup(const char* filepath);
tulip_logger_state tulip_logging_teardown(tulip_logger_state logger);

void tulip_log_message(const char** tags, const char* message);
void tulip_log_warning(const char** tags, const char* warning);
void tulip_log_error(const char** tags, const char* error);
