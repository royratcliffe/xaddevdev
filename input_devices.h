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
  char *lazy_name;
};

/*!
 * \brief Find an input device by its entry name.
 * \param name The entry name of the input device to find.
 * \return Pointer to the input device if found, or \c NULL if not found.
 */
struct input_device *find_input_device(const char *name);

/*!
 * \brief Add a new input device.
 * \param name The entry name of the input device to add.
 * \param fd The file descriptor of the input device.
 * \return Pointer to the newly added input device, or \c NULL on failure.
 */
struct input_device *add_input_device(const char *name, int fd);

/*!
 * \brief Get the name of an input device.
 * \param device The input device to get the name of.
 * \return The name of the input device.
 * \details This function returns the name of the input device. If the device has
 * a lazy name, it will be returned. Otherwise, the name stored in the cons cell
 * will be returned.
 *
 * This means that the name will be either the EVIOC name obtained from the
 * device's properties or the directory entry name used when adding the device
 * to the list. The function ensures that a valid name is returned for the input
 * device, allowing for proper identification and management of input events
 * associated with the device.
 *
 * \note The returned name may not be unique among all input devices managed by
 * the system, so it should be used with caution when identifying devices.
 */
const char *input_device_name(const struct input_device *device);

/*!
 * \brief Remove an input device by its entry name.
 * \param name The entry name of the input device to remove.
 * \retval 0 on success.
 * \retval -ENOENT if the input device with the specified name was not found.
 * \retval -EIO if an I/O error occurred while closing the input device's file
 * descriptor.
 */
int remove_input_device(const char *name);

/*!
 * \brief Find an input device for a given epoll event.
 * \param event The epoll event to find the input device for.
 * \return Pointer to the input device if found, or \c NULL if not found.
 */
struct input_device *find_input_device_for_event(const struct epoll_event *event);

/*!
 * \brief Read an input event from an input device associated with an epoll event.
 * \param event The epoll event associated with the input device.
 * \param input_event The input event structure to store the read event.
 * \retval 0 on success.
 * \retval -ENOENT if the input device for the event was not found.
 * \retval -EIO if an I/O error occurred while reading the input event.
 */
int read_input_device_for_event(const struct epoll_event *event, struct input_event *input_event);
