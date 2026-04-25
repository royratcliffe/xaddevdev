/* SPDX-License-Identifier: MIT */

/*!
 * \file epoll.c
 * \brief Implementation of epoll monitoring functions for managing file descriptors.
 * \details This file provides functions to create an epoll instance, add,
 * modify, and delete file descriptors from the epoll instance, wait for events,
 * and close the epoll instance. Each function handles errors by returning the
 * negative errno value, allowing the caller to handle errors appropriately.
 */
#include "epoll.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int create_epoll(struct epoll *epoll, int maxevents) {
  const int fd = epoll_create1(0);
  if (fd < 0) {
    return -1;
  }
  epoll->events = malloc(sizeof(struct epoll_event) * maxevents);
  if (epoll->events == NULL) {
    /*
     * Capture the error code before closing the file descriptor, as close() may
     * modify errno. Return the malloc() error code to the caller to indicate
     * the failure, allowing it to handle the error appropriately.
     */
    const int err = errno;
    (void)close(fd);
    errno = err;
    return -2;
  }
  epoll->fd = fd;
  epoll->maxevents = maxevents;
  return 0;
}

int add_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data) {
  struct epoll_event event;
  event.events = events;
  event.data = data;
  return epoll_ctl(epoll->fd, EPOLL_CTL_ADD, fd, &event);
}

int modify_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data) {
  struct epoll_event event;
  event.events = events;
  event.data = data;
  return epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &event);
}

int delete_epoll_event(struct epoll *epoll, int fd) { return epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, NULL); }

int wait_for_epoll_events(struct epoll *epoll, int timeout) { return epoll_wait(epoll->fd, epoll->events, epoll->maxevents, timeout); }

int close_epoll(struct epoll *epoll) {
  int rc = 0;
  if (epoll->fd >= 0) {
    rc = close(epoll->fd);
    epoll->fd = -1;
  }
  /*
   * Freeing NULL is safe and has no effect.
   */
  free(epoll->events);
  epoll->events = NULL;
  return rc;
}
