#include "epoll.h"
#include "when.h"

#include <stdlib.h>

int main(int argc, char *argv[]) {
  struct epoll epoll;
  int rc = create_epoll(&epoll, 10);
  if (rc < 0) {
    return rc;
  }
  OCCURS(epoll, &epoll);

  while ((rc = wait_for_epoll_events(&epoll, 1000)) >= 0) {
    for (int i = 0; i < rc; i++) {
      OCCURS(epoll_event, epoll.events + i);
    }
  }
  if (rc < 0) {
    return rc;
  }

  close_epoll(&epoll);
  return EXIT_SUCCESS;
}
