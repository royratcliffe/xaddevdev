#include "cons.h"
#include "epoll.h"

/*
 * Forward declaration of struct input_event to avoid circular dependency
 * between input_devices.h and linux/input.h. The full definition of struct
 * input_event is not needed in this header file, as it is only used as a
 * pointer in the function declarations. The complete definition of struct
 * input_event will be provided in linux/input.h, which can be included in the
 * implementation file (input_devices.c) where the functions that use struct
 * input_event are defined.
 */
struct input_event;

/*!
 * \brief Input device cell structure for managing input devices in a linked list.
 * \details This structure represents an input device and is designed to be used
 * as a cell in a linked list of input devices. It contains a cons cell for
 * linking devices together, a file descriptor for the input device, and a name
 * string for identifying the device. The cons cell allows the input devices to
 * be stored in a linked list, while the file descriptor and name provide the
 * necessary information to manage and identify each input device.
 */
struct input_device {
  /*!
   * \brief Cons cell for linking input devices in a linked list.
   * \details This cons cell is used to link input devices together in a linked
   * list. It allows for efficient insertion, removal, and traversal of input
   * devices in the list. The cons cell contains a pointer to the next input
   * device in the list, enabling the creation of a chain of input devices that
   * can be easily managed and accessed.
   */
  struct cons cons;

  /*!
   * \brief File descriptor for the input device.
   * \details This file descriptor is used to read events from the input device.
   */
  int fd;

  /*!
   * \brief Event name of the input device.
   * \details This string is used to identify the input device. It may not be
   * unique among all input devices managed by the system. The name is obtained
   * from the device's properties.
   * \note It is important to ensure that the name is properly allocated and freed to
   * avoid memory leaks. The name can be used to find the corresponding input
   * device in the list of input devices, allowing for efficient management of
   * input events associated with each device.
   */
  char *name;
};

struct input_device *find_input_device(const char *name);

struct input_device *add_input_device(const char *name, int fd);

int remove_input_device(const char *name);

struct input_device *find_input_device_for_event(const struct epoll_event *event);

int read_input_device_for_event(const struct epoll_event *event, struct input_event *input_event);
