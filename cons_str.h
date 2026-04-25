/* SPDX-License-Identifier: MIT */

/*!
 * \file cons_str.h
 * \brief Header file for string-based cons cell operations.
 * \details This header declares functions for working with cons cells that
 * store strings in their \c car field. It includes functions for finding a cons
 * cell by string, adding a new cons cell with a string, and removing a cons
 * cell by string. The implementation of these functions is provided in the
 * corresponding \c cons_str.c file.
 */
#ifndef CONS_STR_H
#define CONS_STR_H

#include <cons.h>

/*!
 * \brief Accessor for the \c car field of a cons cell, returning it as a string.
 * \param cell The cons cell to access.
 * \return The string stored in the \c car field of the cons cell.
 * \details This function retrieves the value stored in the \c car field of the
 * given cons cell and returns it as a pointer to a character string.
 */
const char *cons_str_car(const struct cons *cell);

/*!
 * \brief Finds the first cons cell in a list whose \c car field matches the
 * specified string.
 * \param list Pointer to the list head to search through.
 * \param str The string to match in the \c car field of the cons cells.
 * \return Pointer to the matching cons cell pointer, or \c NULL if not found.
 */
struct cons **cons_find_str(struct cons **list, const char *str);

/*!
 * \brief Prepends a cons cell containing a copy of the specified string to a
 * list.
 * \param list Pointer to the list head to which the new cell will be prepended.
 * \param cell The cons cell to prepend to the list.
 * \param str The string to copy and store in the \c car field.
 * \return Pointer to the new list head, or \c NULL if memory allocation fails.
 * \sa cons_remove_str
 */
struct cons **cons_str(struct cons **list, struct cons *cell, const char *str);

/*!
 * \brief Removes the first cons cell in a list whose \c car field matches the
 * specified string.
 * \param list Pointer to the list head from which the cell will be removed.
 * \param str The string to match in the \c car field.
 * \return Pointer to the removed cons cell, or \c CONS_NIL if not found.
 * \note This function frees the memory allocated for the string in the \c car
 * field of the removed cell. The caller is responsible for managing the memory
 * of the removed cell itself if necessary.
 */
struct cons *cons_remove_str(struct cons **list, const char *str);

#endif /* CONS_STR_H */
