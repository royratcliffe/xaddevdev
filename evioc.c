/* SPDX-License-Identifier: MIT */

/*!
 * \file evioc.c
 * \brief Implementation of the function to duplicate the name of an input
 * device using the EVIOCGNAME ioctl request.
 */
#include "evioc.h"

#include <linux/input.h>
#include <string.h>

char *dup_evioc_name(int fd) {
  char buf[256];
  return ioctl(fd, EVIOCGNAME(sizeof(buf)), buf) < 0 ? NULL : strdup(buf);
}
