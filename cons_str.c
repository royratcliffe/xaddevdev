/* SPDX-License-Identifier: MIT */

/*!
 * \file cons_str.c
 * \brief Implementation of string-based cons cell operations.
 */
#include "cons_str.h"

#include <stdlib.h>
#include <string.h>

static bool strcmp_p(struct cons **, struct cons *cell, void *user) { return strcmp(cons_str_car(cell), (const char *)user) == 0; }

const char *cons_str_car(const struct cons *cell) { return (const char *)cons_car(cell); }

struct cons **cons_find_str(struct cons **list, const char *str) { return cons_loop(list, strcmp_p, (void *)str); }

struct cons **cons_str(struct cons **list, struct cons *cell, const char *str) {
  char *copy = strdup(str);
  if (copy == NULL) {
    return NULL;
  }
  cons_rplaca(cell, (void *)copy);
  return cons(list, cell);
}

struct cons *cons_remove_str(struct cons **list, const char *str) {
  struct cons **found = cons_find_str(list, str);
  if (found == NULL) {
    return CONS_NIL;
  }
  struct cons *removed = *found;
  free(cons_car(removed));
  *found = cons_cdr(removed);
  return removed;
}
