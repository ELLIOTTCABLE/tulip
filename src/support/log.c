#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#include "support/log.h"

FILE* logfile;

tulip_logger_state tulip_logging_setup(const char* filepath) {
  logfile = fopen(filepath, "a");

  if (logfile) {
    return (tulip_logger_state){tulip_logger_running, 0, logfile, tulip_logger_default_options};
  } else {
    return (tulip_logger_state){tulip_logger_failure, errno, logfile, tulip_logger_default_options};
  }
}

void tulip_log_message(const char** tags, const char* message) {}
void tulip_log_warning(const char** tags, const char* warning) {}
void tulip_log_error(const char** tags, const char* error) {}

tulip_logger_state tulip_logging_teardown(tulip_logger_state logger) {
   
}
