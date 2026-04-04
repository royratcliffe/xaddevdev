#include "inotify.h"
#include "when.h"

#include <errno.h>
#include <unistd.h>

#include <sys/inotify.h>

int inotify_events_occur(int fd) {
  /*
   * Align the buffer to the alignment requirements of struct inotify_event to
   * ensure that the events are correctly read and processed. The buffer size is
   * set to 1024 bytes, which is sufficient to hold multiple inotify events at
   * once, reducing the number of read system calls needed to process events.
   * The read loop continues until there are no more events to read (indicated
   * by EAGAIN), ensuring that all events are processed without missing any. If
   * an error occurs during reading (other than EAGAIN), an appropriate error
   * code is returned.
   */
  char buffer[1024] __attribute__((aligned(__alignof__(struct inotify_event))));
  ssize_t length;
  while ((length = read(fd, buffer, sizeof(buffer))) > 0) {
    size_t offset = 0;
    while (offset < length) {
      struct inotify_event *event = (struct inotify_event *)(buffer + offset);
      OCCURS(inotify_event, event);
      offset += sizeof(struct inotify_event) + event->len;
    }
  }
  /*
   * Always drain the inotify file descriptor to ensure no missed events. If the
   * read fails with EAGAIN, it signals that all available events have been
   * read, which is expected. Any other error indicates a problem with reading
   * from the inotify file descriptor; return an error code accordingly.
   */
  return length < 0 && errno != EAGAIN ? -errno : 0;
}
