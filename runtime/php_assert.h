#pragma once

#include <cstdio>
#include <signal.h>
#include <unistd.h>

extern int die_on_fail;

extern const char *engine_tag;
extern const char *engine_pid;

extern int php_disable_warnings;
extern int php_warning_level;

void php_warning(char const *message, ...) __attribute__ ((format (printf, 1, 2)));

void php_assert__(const char *msg, const char *file, int line) __attribute__((noreturn));

#define php_assert(EX) do {                          \
  if (!(EX)) {                                       \
    php_assert__ (#EX, __FILE__, __LINE__);          \
  }                                                  \
} while(0)

#define php_critical_error(format, ...) do {                                                              \
  php_warning ("Critical error \"" format "\" in file %s on line %d", ##__VA_ARGS__, __FILE__, __LINE__); \
  raise (SIGUSR2);                                                                                        \
  fprintf (stderr, "_exiting in php_critical_error\n");                                                   \
  _exit (1);                                                                                              \
} while(0)

