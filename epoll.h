/* SPDX-License-Identifier: MIT */

/*!
 * \file epoll.h
 * \brief Header file for epoll monitoring functions for managing file descriptors.
 * \details This header file declares functions to create an epoll instance,
 * add, modify, and delete file descriptors from the epoll instance, wait for
 * events, and close the epoll instance. Each function is designed to handle
 * errors by returning the negative errno value, allowing the caller to handle
 * errors appropriately. The epoll instance is represented by the struct epoll,
 * which contains the file descriptor for the epoll instance, an array of
 * epoll_event structures to store events, and the maximum number of events that
 * can be stored in the array.
 */
#ifndef EPOLL_H_
#define EPOLL_H_

#include <sys/epoll.h>

/*!
 * \brief A file descriptor monitoring set using Linux event polling.
 * \note epoll_ctl() performs control operations on the epoll instance referred
 * to by the file descriptor "epfd". The operation specified by "op" is applied
 * to the target file descriptor "fd" (which must have been registered with the
 * epoll instance in the case of EPOLL_CTL_MOD and EPOLL_CTL_DEL operations).
 * The "event" argument describes the object linked to the file descriptor "fd"
 * and the events that will be monitored for that file descriptor.
 */
struct epoll {
  int fd;
  struct epoll_event *events;
  int maxevents;
};

/*!
 * \brief Creates an epoll monitoring instance.
 * \param epoll The epoll instance to initialise.
 * \param maxevents The maximum number of events to poll at a time.
 * \retval 0 on success.
 * \retval -1 on failure to create the epoll instance (e.g., if epoll_create1 fails).
 * \retval -2 on failure to allocate memory for the events array (e.g., if malloc fails).
 * \details This function creates an epoll instance and stores the epoll
 * monitoring descriptor in the provided epoll structure. It uses the
 * epoll_create1 system call to create the instance. If the creation fails, it
 * logs an error message and returns the negative errno value. On success, it
 * allocates memory for the events array based on the specified maximum number
 * of events and returns 0. If memory allocation fails, it logs an error message
 * and returns the negative errno value.
 */
int create_epoll(struct epoll *epoll, int maxevents);

/*!
 * \brief Adds a file descriptor to the epoll instance with specified events and data.
 * \param epoll The epoll instance to which the file descriptor will be added.
 * \param fd The file descriptor to add to the epoll instance.
 * \param events The events to monitor for the file descriptor (e.g., EPOLLIN, EPOLLOUT).
 * \param data The user data to associate with the file descriptor in the epoll instance.
 * \return 0 on success, or negative with errno set on failure.
 * \details This function creates an epoll_event structure with the specified
 * events and data, and then calls epoll_ctl with the EPOLL_CTL_ADD operation to
 * add the file descriptor to the epoll instance. If the operation fails, it
 * logs an error message and returns the negative errno value. On success, it
 * returns 0.
 */
int add_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data);

/*!
 * \brief Modifies the events and data associated with a file descriptor in the epoll instance.
 * \param epoll The epoll instance containing the file descriptor to modify.
 * \param fd The file descriptor to modify in the epoll instance.
 * \param events The new events to monitor for the file descriptor (e.g., EPOLLIN, EPOLLOUT).
 * \param data The new user data to associate with the file descriptor in the epoll instance.
 * \return 0 on success, or negative with errno set on failure.
 * \details This function creates an epoll_event structure with the specified
 * events and data, and then calls epoll_ctl with the EPOLL_CTL_MOD operation to
 * modify the existing file descriptor in the epoll instance. If the operation
 * fails, it logs an error message and returns the negative errno value. On
 * success, it returns 0.
 */
int modify_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data);

/*!
 * \brief Deletes a file descriptor from the epoll instance.
 * \param epoll The epoll instance containing the file descriptor to delete.
 * \param fd The file descriptor to delete from the epoll instance.
 * \return 0 on success, or negative with errno set on failure.
 * \details This function calls epoll_ctl with the EPOLL_CTL_DEL operation to
 * remove the specified file descriptor from the epoll instance. If the
 * operation fails, it returns the negative errno value.
 */
int delete_epoll_event(struct epoll *epoll, int fd);

/*!
 * \brief Wait for events on an epoll instance.
 * \param epoll The epoll instance to wait on.
 * \param timeout The maximum time to wait in milliseconds, or -1 to wait indefinitely.
 * \return The number of file descriptors ready for the requested I/O, or
 * negative with errno set on error.
 */
int wait_for_epoll_events(struct epoll *epoll, int timeout);

/*!
 * \brief Closes an epoll instance and frees its associated resources.
 * \param epoll The epoll instance to close.
 * \return 0 on success, or the close() return value on failure.
 * \details This function closes the file descriptor associated with the epoll
 * instance and frees the memory allocated for the events array. It checks if
 * the file descriptor is valid before attempting to close it, and logs an error
 * message if the close operation fails. After closing the file descriptor, it
 * sets the fd to -1 to indicate that it is no longer valid. Finally, it frees
 * the events array and sets the pointer to NULL to prevent dangling pointers.
 * \note Closing works idempotently, so if the file descriptor is already closed
 * (i.e., fd is -1), the function will simply free the events array and return 0
 * without attempting to close an invalid file descriptor, which helps to
 * prevent errors and ensures that resources are properly released even if the
 * close_epoll function is called multiple times on the same epoll instance.
 * This design allows for safer resource management and helps to avoid issues
 * with double-closing file descriptors or accessing freed memory.
 */
int close_epoll(struct epoll *epoll);

#endif /* EPOLL_H_ */
