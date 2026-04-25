/* SPDX-License-Identifier: MIT */

/*!
 * \file evioc.h
 * \brief Header file for evioc.c, which provides a function to duplicate the
 * name of an input device using the EVIOCGNAME ioctl request.
 */
#ifndef EVIOC_H
#define EVIOC_H

/*!
 * \brief Duplicates the name of an input device using the EVIOCGNAME ioctl request.
 * \param fd The file descriptor of the input device.
 * \return A newly allocated string containing the name of the input device, or
 * \c NULL if the ioctl request fails.
 * \details This function uses the EVIOCGNAME ioctl request to retrieve the name
 * of the input device associated with the given file descriptor. It stores the
 * name in a buffer and then duplicates it using \c strdup to return a newly
 * allocated string. If the ioctl request fails (e.g., due to insufficient
 * permissions or an invalid file descriptor), the function returns \c NULL to
 * indicate the failure. The caller is responsible for freeing the returned
 * string when it is no longer needed.
 */
char *dup_evioc_name(int fd);

#endif /* EVIOC_H */
