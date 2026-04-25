/* SPDX-License-Identifier: MIT */

/*!
 * \file pr.h
 * \details Header file for the pr logging functions. This file declares the
 * pr_verbosity variable, which controls the verbosity level of the logging, and
 * the pr_log function, which is used to log messages at different verbosity
 * levels. The pr_log function takes a log level, a format string, and a
 * variable argument list, and outputs the log message to the appropriate output
 * stream based on the log level. The pr_log function also checks the current
 * verbosity level before outputting the log message, allowing for more or less
 * detailed logging based on the needs of the developer. The pr_log function can
 * be used to log informational messages, debug messages, warnings, and errors,
 * providing a consistent and flexible way to output log messages throughout the
 * program. The implementation of the pr_log function can be designed to write
 * log messages to standard output, standard error, or to a log file, depending
 * on the requirements of the project and the desired logging behaviour.
 */
#ifndef PR_H_
#define PR_H_

#include <stdarg.h>
#include <stdbool.h>

#define pr(level, ...)             \
  do {                             \
    if (pr_logging(level)) {       \
      pr_log_level(level);         \
      pr_log_errno(level);         \
      pr_logf(level, __VA_ARGS__); \
    }                              \
  } while (0)

#define pr_err(...) pr(pr_level_err, __VA_ARGS__)
#define pr_warn(...) pr(pr_level_warn, __VA_ARGS__)
#define pr_info(...) pr(pr_level_info, __VA_ARGS__)
#define pr_debug(...) pr(pr_level_debug, __VA_ARGS__)

enum pr_level {
  pr_level_err = 0,
  pr_level_warn = 1,
  pr_level_info = 2,
  pr_level_debug = 3,
  pr_level_max = 4,
};

bool pr_logging(enum pr_level level);

int pr_log(enum pr_level level, const char *format, va_list args);
int pr_logf(enum pr_level level, const char *format, ...);

int pr_log_level(enum pr_level level);
int pr_log_errno(enum pr_level level);

void pr_verbosity_set(int verbosity);
void pr_verbosity_inc(void);
void pr_verbosity_dec(void);

#endif /* PR_H_ */
