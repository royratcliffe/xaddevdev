/* SPDX-License-Identifier: MIT */

#include "epoll.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define pr_err(...) fprintf(stderr, ##__VA_ARGS__)

int create_epoll(struct epoll *epoll, int maxevents) {
  const int fd = epoll_create1(0);
  if (fd < 0) {
    pr_err("Failed to create epoll: %d (%s)\n", errno, strerror(errno));
    return -errno;
  }
  epoll->events = malloc(sizeof(struct epoll_event) * maxevents);
  if (epoll->events == NULL) {
    pr_err("Failed to allocate memory for epoll events: %d (%s)\n", errno, strerror(errno));
    close(fd);
    return -errno;
  }
  epoll->fd = fd;
  epoll->maxevents = maxevents;
  return 0;
}

int add_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data) {
  struct epoll_event event;
  event.events = events;
  event.data = data;
  int rc = epoll_ctl(epoll->fd, EPOLL_CTL_ADD, fd, &event);
  if (rc < 0) {
    pr_err("Failed to add file descriptor %d to epoll: %d (%s)\n", fd, errno, strerror(errno));
    rc = -errno;
  }
  return rc;
}

int modify_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data) {
  struct epoll_event event;
  event.events = events;
  event.data = data;
  int rc = epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &event);
  if (rc < 0) {
    pr_err("Failed to modify file descriptor %d in epoll: %d (%s)\n", fd, errno, strerror(errno));
    rc = -errno;
  }
  return rc;
}

int delete_epoll_event(struct epoll *epoll, int fd) {
  int rc = epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, NULL);
  if (rc < 0) {
    pr_err("Failed to delete file descriptor %d from epoll: %d (%s)\n", fd, errno, strerror(errno));
    rc = -errno;
  }
  return rc;
}

int wait_for_epoll_events(struct epoll *epoll, int timeout) {
  int rc = epoll_wait(epoll->fd, epoll->events, epoll->maxevents, timeout);
  if (rc < 0) {
    pr_err("Failed to wait for epoll events: %d (%s)\n", errno, strerror(errno));
    rc = -errno;
  }
  return rc;
}

void close_epoll(struct epoll *epoll) {
  if (epoll->fd >= 0) {
    if (close(epoll->fd) < 0) {
      pr_err("Failed to close epoll file descriptor: %d (%s)\n", errno, strerror(errno));
    }
    epoll->fd = -1;
  }
  free(epoll->events);
  epoll->events = NULL;
}
