#include "xaddevdev.h"
#include "epoll.h"
#include "pr.h"
#include "version.h"
#include "when.h"

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>

static struct epoll epoll;

struct epoll *xaddevdev_epoll(void) { return &epoll; }

int main(int argc, char *argv[]) {
  static struct option longopts[] = {{"version", no_argument, NULL, 'V'},
                                     {"verbose", no_argument, NULL, 'v'},
                                     {"host", required_argument, NULL, 'h'},
                                     {"port", required_argument, NULL, 'p'},
                                     {"help", no_argument, NULL, '?'},
                                     {
                                         NULL,
                                     }};
  int c, longind;
  while ((c = getopt_long(argc, argv, "Vvh:p:?", longopts, &longind)) >= 0) {
    switch (c) {
    case 0:
      OCCURS(long_opt, longopts + longind, optarg);
      break;
    case 'V':
      pr_info("xaddevdev version %s\n", VERSION);
      return EXIT_SUCCESS;
    case 'v':
      pr_verbosity++;
      break;
    case 'h':
      OCCURS(opt_h, optarg);
      break;
    case 'p':
      OCCURS(opt_p, optarg);
      break;
    case '?':
      pr_info("Usage: %s [OPTIONS]\n", argv[0]);
      pr_info("Options:\n");
      pr_info("  -V, --version          Show version information and exit\n");
      pr_info("  -v, --verbose          Increase verbosity level\n");
      pr_info("  -h, --host=HOST        Connect to Redis server at HOST\n");
      pr_info("  -p, --port=PORT        Connect to Redis server at PORT\n");
      pr_info("  -?, --help             Show this help message and exit\n");
      return EXIT_SUCCESS;
    default:
      pr_warn("Unknown option: 0%o\n", c);
      return EXIT_FAILURE;
    }
  }
  if (optind < argc) {
    pr_err("Unexpected non-option argument: %s\n", argv[optind]);
    return EXIT_FAILURE;
  }
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
