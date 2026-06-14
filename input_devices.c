/*
 * SPDX-FileCopyrightText: 2024, Roy Ratcliffe, Northumberland, United Kingdom
 * SPDX-License-Identifier: MIT
 */

/*!
 * \file input_devices.c
 * \brief Implementation of input device management functions for handling input events.
 * \details This file provides functions to manage input devices, including
 * finding, adding, and removing input devices, as well as reading input events
 * from devices. The functions utilise a linked list of input devices, where
 * each device is represented by a structure containing a cons cell for linking
 * devices together, a file descriptor for the device, and an device name string
 * for identifying the device. The implementation ensures proper memory
 * management and error handling, allowing for efficient management of input
 * devices and their associated events.
 */
#include "input_devices.h"

#include "cons_str.h"
#include "containerof.h"
#include "evioc.h"
#include "pr.h"

#include <errno.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

static struct cons *input_devices = CONS_NIL;

static struct input_device *input_device(struct cons *cell) { return cell != NULL ? containerof(cell, struct input_device, cons) : NULL; }

static bool fd_p(struct cons **, struct cons *cell, void *user) { return input_device(cell)->fd == (int)(intptr_t)user; }

static struct input_device *find_input_device_for_fd(int fd) {
  struct cons **found = cons_loop(&input_devices, fd_p, (void *)(intptr_t)fd);
  return found != NULL ? input_device(*found) : NULL;
}

struct input_device *find_input_device(const char *name) {
  struct cons **found = cons_find_str(&input_devices, name);
  return found != NULL ? input_device(*found) : NULL;
}

struct input_device *add_input_device(const char *name, int fd) {
  struct input_device *device = malloc(sizeof(struct input_device));
  if (device == NULL) {
    return NULL;
  }
  device->fd = fd;
  device->lazy_name = dup_evioc_name(fd);
  /*
   * Ignore input devices that do not have a name, as they cannot be identified.
   * In this case, free the allocated memory for the device structure and return
   * NULL to indicate that the device could not be added. This ensures that only
   * "valid" input devices with identifiable names are managed in the list of
   * input devices.
   */
  if (device->lazy_name == NULL) {
    free(device);
    return NULL;
  }
  if (cons_str(&input_devices, &device->cons, name) == NULL) {
    free(device->lazy_name);
    free(device);
    return NULL;
  }
  return device;
}

const char *input_device_name(const struct input_device *device) { return device->lazy_name != NULL ? device->lazy_name : cons_str_car(&device->cons); }

int remove_input_device(const char *name) {
  struct cons *cell = cons_remove_str(&input_devices, name);
  if (cell == CONS_NIL) {
    return -ENOENT;
  }
  struct input_device *device = input_device(cell);
  /*
   * Removing the input device from the list automatically closes the device's
   * file descriptor. In this way, the list of input devices serves as the
   * single source of truth for managing the lifecycle of input device file
   * descriptors, ensuring that they are properly closed when an input device is
   * removed from the list.
   */
  int rc = 0;
  if (device->fd >= 0) {
    rc = close(device->fd);
  }
  free(device->lazy_name);
  free(device);
  return rc;
}

struct input_device *find_input_device_for_event(const struct epoll_event *event) {
  struct input_device *device = (struct input_device *)event->data.ptr;
  if (cons_find(&input_devices, &device->cons) == NULL) {
    return NULL;
  }
  return device;
}

int read_input_device_for_event(const struct epoll_event *event, struct input_event *input_event) {
  struct input_device *device = find_input_device_for_event(event);
  if (device == NULL) {
    return -ENOENT;
  }
  ssize_t bytes_read = read(device->fd, input_event, sizeof(*input_event));
  if (bytes_read < 0) {
    return -EIO;
  }
  return 0;
}
