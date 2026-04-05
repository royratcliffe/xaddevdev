#include <cons.h>

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
