#include "epoll.h"
#include "inotify.h"
#include "pr.h"
#include "when.h"

#include <sys/inotify.h>

static int inotify_fd = -1;

CAUSES(epoll, init_inotify) {
  struct epoll *epoll = (struct epoll *)with;
  inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (inotify_fd < 0) {
    pr_err("Failed to initialise inotify: %d (%s)\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  int rc = add_epoll_event(epoll, inotify_fd, EPOLLIN, (epoll_data_t){.fd = inotify_fd});
  if (rc < 0) {
    pr_err("Failed to add inotify file descriptor to epoll: %d (%s)\n", -rc, strerror(-rc));
    close(inotify_fd);
    inotify_fd = -1;
    exit(EXIT_FAILURE);
  }
  OCCURS(inotify, &inotify_fd);
}

CAUSES(epoll_event, handle_inotify_events) {
  struct epoll_event *ep_event = (struct epoll_event *)with;
  if (ep_event->data.fd != inotify_fd || !(ep_event->events & EPOLLIN)) {
    return;
  }
  if (inotify_events_occur(inotify_fd) < 0) {
    pr_err("Failed to read from inotify file descriptor: %d (%s)\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}
