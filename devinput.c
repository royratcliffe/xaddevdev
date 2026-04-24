#include "epoll.h"
#include "input_devices.h"
#include "pr.h"
#include "when.h"
#include "xaddevdev.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/inotify.h>

#define DEV_INPUT_PATH "/dev/input"

static int scan_input_devices(const char *dirname, struct epoll *epoll);

CAUSES(epoll, devinput_epoll) {
  pr_info("Directory: %s\n", DEV_INPUT_PATH);
  int rc = scan_input_devices(DEV_INPUT_PATH, xaddevdev_epoll());
  if (rc < 0) {
    pr_err("Failed to scan input devices: %d (%s)\n", -rc, strerror(-rc));
    exit(-rc);
  }
}

/*
 * Handle epoll events for input devices. When an epoll event occurs, the
 * handler decodes the event to determine which input device it corresponds to
 * and what type of event occurred. The epoll event must contain a pointer to
 * the input device associated with the event, which allows the handler to
 * identify the specific device that triggered the event.
 */
CAUSES(epoll_event, devinput_epoll_event) {
  /*
   * Decode the epoll event to determine which input device it corresponds to
   * and what type of event occurred. The epoll event must contain a pointer to
   * the input device associated with the event, which allows the handler to
   * identify the specific device that triggered the event. By checking the
   * event's flags (e.g., EPOLLIN), the handler can determine if there is data
   * to read from the device, and then we can handle the event accordingly, such
   * as reading input data or processing the event in some way.
   *
   * Do not assume that the event's data pointer matches an input device in the
   * list, as the event may have been triggered by a device that has already
   * been removed from the list, or may be some other non-related input device
   * that is not being tracked. In such cases, the event should be ignored to
   * prevent potential errors or undefined behaviour that could arise from
   * attempting to access a non-existent or unrelated input device.
   */
  struct epoll_event *event = (struct epoll_event *)with;
  if (!(event->events & EPOLLIN)) {
    return;
  }
  struct input_device *device = find_input_device_for_event(event);
  if (device == NULL) {
    return;
  }
  struct input_event input_event;
  int rc = read_input_device_for_event(event, &input_event);
  if (rc < 0) {
    pr_warn("Failed to read input event for device %s: %d (%s)\n", device->name, -rc, strerror(-rc));
    return;
  }
  // pr_info("Input event: device=%s type=%u code=%u value=%d\n", device->name, input_event.type, input_event.code, input_event.value);
  OCCURS(input_event, &input_event, device->name);
}

/*
 * The inotify event handler is responsible for monitoring the /dev/input
 * directory for changes, such as the addition or removal of input devices. When
 * an inotify event occurs, the handler checks if the event is related to a
 * directory (ignored) or a character device (handled). For file events, it
 * logs the details of the event, including the watch descriptor, event mask,
 * cookie, length of the name, and the name of the file associated with the
 * event. This allows us to track changes in the input devices and respond
 * accordingly, such as adding new devices to the epoll instance or removing
 * devices that are no longer present.
 */
CAUSES(inotify, devinput_inotify) {
  int inotify_fd = *(int *)with;
  if (inotify_add_watch(inotify_fd, DEV_INPUT_PATH, IN_ALL_EVENTS) < 0) {
    pr_err("Failed to add inotify watch: %d (%s)\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

CAUSES(inotify_event, devinput_inotify_event) {
  struct inotify_event *event = (struct inotify_event *)with;
  if (event->mask & IN_ISDIR) {
    return;
  }
  pr_debug("Inotify event: wd=%d mask=0x%08x cookie=%u len=%u name=%s\n", event->wd, event->mask, event->cookie, event->len, event->name);
  if (event->mask & IN_CREATE) {
    /*
     * The newly-created entry in the /dev/input directory may not be an input
     * device. In such cases, do not attempt to add the device to the list of
     * input devices or to the epoll instance, as this could lead to errors or
     * undefined behaviour if the device is not properly handled.
     */
    pr_info("Input device created: %s\n", event->name);
    (void)scan_input_devices(DEV_INPUT_PATH, xaddevdev_epoll());
  } else if (event->mask & IN_DELETE) {
    pr_info("Input device deleted: %s\n", event->name);
    struct input_device *device = find_input_device(event->name);
    if (device == NULL) {
      return;
    }
    delete_epoll_event(xaddevdev_epoll(), device->fd);
    remove_input_device(event->name);
  }
}

int scan_input_devices(const char *dirname, struct epoll *epoll) {
  DIR *dir = opendir(dirname);
  if (dir == NULL) {
    pr_err("Failed to open directory %s: %d (%s)\n", dirname, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  for (struct dirent *entry; (entry = readdir(dir)) != NULL;) {
    if (entry->d_type != DT_CHR) {
      continue;
    }
    struct input_device *input_device = find_input_device(entry->d_name);
    if (input_device != NULL) {
      continue;
    }
    /*
     * O_NONBLOCK | O_CLOEXEC
     *
     * The following fails if the device is already opened by another process
     * without O_NONBLOCK, but this is a common scenario for input devices, and
     * we can handle it by simply skipping the device.
     *
     * The following fails without permissions. The process needs to possess
     * read permissions for the device, which is typically granted to users in
     * the "input" group. If the process does not have the necessary
     * permissions, it will fail to open the device; in which case, handle it by
     * simply skipping the device.
     */
    int fd = openat(dirfd(dir), entry->d_name, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
      pr_info("Failed to open input device %s/%s: %d (%s)\n", dirname, entry->d_name, errno, strerror(errno));
      continue;
    }

    if ((input_device = add_input_device(entry->d_name, fd)) == NULL) {
      pr_warn("Failed to add input device %s: %d (%s)\n", entry->d_name, errno, strerror(errno));
      (void)close(fd);
      continue;
    }
    pr_info("Entry: %s\n", entry->d_name);

    if (add_epoll_event(epoll, fd, EPOLLIN, (epoll_data_t){.ptr = input_device}) < 0) {
      pr_warn("Failed to add epoll event for device %s/%s: %d (%s)\n", dirname, entry->d_name, errno, strerror(errno));
      remove_input_device(entry->d_name);
      continue;
    }
    pr_info("Input device name: %s\n", entry->d_name);
  }
  closedir(dir);
  return 0;
}
