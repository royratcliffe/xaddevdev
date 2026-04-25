#include "epoll.h"
#include "inotify.h"
#include "pr.h"
#include "when.h"

#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

static int inotify_fd = -1;

/*
 * Initialise inotify and add its file descriptor to the epoll instance.
 * This function sets up inotify to monitor file system events and integrates
 * it with the epoll event loop for efficient event handling.
 */
CAUSES(epoll, inotify_epoll) {
  struct epoll *epoll = (struct epoll *)with;
  inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (inotify_fd < 0) {
    pr_err("Failed to initialise inotify\n");
    exit(EXIT_FAILURE);
  }
  int rc = add_epoll_event(epoll, inotify_fd, EPOLLIN, (epoll_data_t){.fd = inotify_fd});
  if (rc < 0) {
    pr_err("Failed to add inotify file descriptor to epoll\n");
    (void)close(inotify_fd);
    inotify_fd = -1;
    exit(EXIT_FAILURE);
  }
  OCCURS(inotify, &inotify_fd);
}

/*
 * Handle inotify events when the inotify file descriptor becomes ready for
 * reading. This function is called when an epoll event occurs for the inotify
 * file descriptor, indicating that there are inotify events to be processed.
 * The function reads the inotify events from the file descriptor and handles
 * them accordingly. If there is an error while reading from the inotify file
 * descriptor, the function logs the error and exits the program, as it
 * indicates a critical failure in the event handling mechanism. By processing
 * the inotify events, the program can respond to changes in the monitored
 * directory (e.g., /dev/input) and manage input devices dynamically as they are
 * added or removed.
 */
CAUSES(epoll_event, inotify_epoll_event) {
  struct epoll_event *ep_event = (struct epoll_event *)with;
  if (ep_event->data.fd != inotify_fd || !(ep_event->events & EPOLLIN)) {
    return;
  }
  if (inotify_events_occur(inotify_fd) < 0) {
    pr_err("Failed to read from inotify file descriptor\n");
    exit(EXIT_FAILURE);
  }
}
