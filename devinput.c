#include "cons_str.h"
#include "containerof.h"
#include "epoll.h"
#include "pr.h"
#include "when.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/inotify.h>
#include <unistd.h>

#define DEV_INPUT_PATH "/dev/input"

struct input_device {
  struct cons cons;
  int fd;
  char *name;
};

static struct cons *input_devices = CONS_NIL;

static struct input_device *input_device(struct cons *cell) { return cell != NULL ? containerof(cell, struct input_device, cons) : NULL; }

static bool fd_p(struct cons **, struct cons *cell, void *user) { return input_device(cell)->fd == (int)(intptr_t)user; }

static struct input_device *find_input_device_for_fd(int fd) {
  struct cons **found = cons_loop(&input_devices, fd_p, (void *)(intptr_t)fd);
  return found != NULL ? input_device(*found) : NULL;
}

static struct input_device *find_input_device(const char *path) {
  struct cons **list = cons_find_str(&input_devices, path);
  return list != NULL ? input_device(*list) : NULL;
}

static struct input_device *add_input_device(const char *path, int fd, const char *name) {
  struct input_device *device = malloc(sizeof(struct input_device));
  if (device == NULL) {
    return NULL;
  }
  device->fd = fd;
  device->name = strdup(name);
  if (device->name == NULL) {
    free(device);
    return NULL;
  }
  if (cons_str(&input_devices, &device->cons, path) == NULL) {
    free(device->name);
    free(device);
    return NULL;
  }
  return device;
}

static void remove_input_device(const char *path) {
  struct cons *cell = cons_remove_str(&input_devices, path);
  if (cell == CONS_NIL) {
    return;
  }
  struct input_device *device = input_device(cell);
  /*
   * Removing the input device from the list automatically closes the device's
   * file descriptor. In this way, the list of input devices serves as the
   * single source of truth for managing the lifecycle of input device file
   * descriptors, ensuring that they are properly closed when an input device is
   * removed from the list.
   */
  if (device->fd >= 0) {
    (void)close(device->fd);
  }
  free(device);
}

static void dir() {
  pr_info("Directory: %s\n", DEV_INPUT_PATH);
  DIR *dir = opendir(DEV_INPUT_PATH);
  if (dir == NULL) {
    pr_err("Failed to open directory %s: %d (%s)\n", DEV_INPUT_PATH, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  closedir(dir);
}

CAUSES(epoll, init_devinput) {
  pr_info("Directory: %s\n", DEV_INPUT_PATH);
  DIR *dir = opendir(DEV_INPUT_PATH);
  if (dir == NULL) {
    pr_err("Failed to open directory %s: %d (%s)\n", DEV_INPUT_PATH, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  for (struct dirent *entry; (entry = readdir(dir)) != NULL;) {
    if (entry->d_type != DT_CHR) {
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
      pr_info("Failed to open input device %s/%s: %d (%s)\n", DEV_INPUT_PATH, entry->d_name, errno, strerror(errno));
      continue;
    }

    char name[BUFSIZ];
    int rc = ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    if (rc < 0) {
      pr_err("Failed to get name for device %s/%s: %d (%s)\n", DEV_INPUT_PATH, entry->d_name, errno, strerror(errno));
      (void)close(fd);
      continue;
    }

    struct input_device *input_device;
    if ((input_device = add_input_device(entry->d_name, fd, name)) == NULL) {
      pr_err("Failed to add input device: %d (%s)\n", errno, strerror(errno));
      (void)close(fd);
      continue;
    }
    pr_info("Entry: %s\n", entry->d_name);

    extern struct epoll epoll;
    if (add_epoll_event(&epoll, fd, EPOLLIN, (epoll_data_t){.ptr = input_device}) < 0) {
      pr_err("Failed to add epoll event for device %s/%s: %d (%s)\n", DEV_INPUT_PATH, entry->d_name, errno, strerror(errno));
      remove_input_device(entry->d_name);
      continue;
    }
    pr_info("Input device name: %s\n", name);
  }
  closedir(dir);
}

/*
 * Handle epoll events for input devices. When an epoll event occurs, the
 * handler decodes the event to determine which input device it corresponds to
 * and what type of event occurred. The epoll event must contain a pointer to
 * the input device associated with the event, which allows the handler to
 * identify the specific device that triggered the event.
 */
CAUSES(epoll_event, handle_epoll_events) {
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
  struct input_device *device = (struct input_device *)event->data.ptr;
  if (cons_find(&input_devices, &device->cons) == NULL) {
    return;
  }
  if (event->events & EPOLLIN) {
    pr_info("Input event on device with fd %d\n", device->fd);
  }
  if (!(event->events & EPOLLIN)) {
    return;
  }
  struct input_event input_event;
  ssize_t bytes_read = read(device->fd, &input_event, sizeof(input_event));
  if (bytes_read < 0) {
    pr_err("Failed to read from input device with fd %d: %d (%s)\n", device->fd, errno, strerror(errno));
    return;
  }
  pr_info("Input event: device=%s type=%u code=%u value=%d\n", device->name, input_event.type, input_event.code, input_event.value);
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
CAUSES(inotify, handle_inotify) {
  int inotify_fd = *(int *)with;
  if (inotify_add_watch(inotify_fd, DEV_INPUT_PATH, IN_ALL_EVENTS) < 0) {
    pr_err("Failed to add inotify watch: %d (%s)\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

CAUSES(inotify_event, handle_input_events) {
  struct inotify_event *event = (struct inotify_event *)with;
  if (event->mask & IN_ISDIR) {
    return;
  }
  pr_err("Input event: wd=%d mask=0x%08x cookie=%u len=%u name=%s\n", event->wd, event->mask, event->cookie, event->len, event->name);
}
