#include "epoll.h"
#include "when.h"

#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  struct epoll epoll;
  int rc = create_epoll(&epoll, 10);
  if (rc < 0) {
    return rc;
  }
  OCCURS(epoll, &epoll);

  /*
   * Allow the event loop to be interrupted by signals, and handle EINTR by
   * simply continuing to wait for events. This allows the program to respond to
   * signals (e.g., for graceful shutdown) without exiting the event loop
   * prematurely. If epoll_wait returns an error other than EINTR, it indicates
   * a more serious issue, and the program should handle it appropriately (e.g.,
   * by logging the error and exiting). By returning -rc, we propagate the error
   * code to the launcher, allowing it to handle the error as needed. If
   * epoll_wait returns a positive number, it indicates the number of file
   * descriptors that are ready for the requested I/O.
   */
  while ((rc = wait_for_epoll_events(&epoll, 1000)) >= 0 || rc == -EINTR) {
    for (int i = 0; i < rc; i++) {
      OCCURS(epoll_event, epoll.events + i);
    }
  }
  if (rc < 0) {
    return -rc;
  }

  close_epoll(&epoll);
  return EXIT_SUCCESS;
}
