#ifndef INOTIFY_H_
#define INOTIFY_H_

/*!
 * \brief Handle inotify events by reading and processing them.
 * \param fd The file descriptor for the inotify instance.
 * \return 0 on success, or -errno on failure.
 * \details This function reads events from the inotify file descriptor and
 * processes them. It uses a buffer that is aligned to the requirements of
 * struct inotify_event to ensure correct processing of events. The function
 * continues to read events until there are no more to read (indicated by
 * EAGAIN). If an error occurs during reading (other than EAGAIN), it returns a
 * negative errno value. The OCCURS macro is used to handle each inotify event,
 * allowing for modular registration of event handlers in a callback framework.
 * This design ensures that all events are processed without missing any, and
 * that any errors encountered during reading are appropriately handled and
 * reported.
 */
int inotify_events_occur(int fd);

#endif /* INOTIFY_H_ */
