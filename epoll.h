/* SPDX-License-Identifier: MIT */

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
 * \return 0 on success, or -errno on failure.
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
 * \return 0 on success, or -errno on failure.
 * \details This function creates an epoll_event structure with the specified
 * events and data, and then calls epoll_ctl with the EPOLL_CTL_ADD operation to
 * add the file descriptor to the epoll instance. If the operation fails, it
 * logs an error message and returns the negative errno value. On success, it
 * returns 0.
 */
int add_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data);

int modify_epoll_event(struct epoll *epoll, int fd, uint32_t events, epoll_data_t data);

int delete_epoll_event(struct epoll *epoll, int fd);

/*!
 * \brief Wait for events on an epoll instance.
 * \param epoll The epoll instance to wait on.
 * \param timeout The maximum time to wait in milliseconds, or -1 to wait indefinitely.
 * \return The number of file descriptors ready for the requested I/O, or -errno on error.
 */
int wait_for_epoll_events(struct epoll *epoll, int timeout);

/*!
 * \brief Closes an epoll instance and frees associated resources.
 * \param epoll The epoll instance to close.
 * \return 0 on success, or -errno on failure.
 * \details This function closes the file descriptor associated with the epoll
 * instance and frees the memory allocated for the events array. It checks if
 * the file descriptor is valid before attempting to close it, and logs an error
 * message if the close operation fails. After closing the file descriptor, it
 * sets the fd to -1 to indicate that it is no longer valid. Finally, it frees
 * the events array and sets the pointer to NULL to prevent dangling pointers.
 */
int close_epoll(struct epoll *epoll);

#endif /* EPOLL_H_ */
