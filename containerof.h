/*
 * SPDX-FileCopyrightText: 2024, Roy Ratcliffe, Northumberland, United Kingdom
 * SPDX-License-Identifier: MIT
 */

#ifndef CONTAINEROF_H
#define CONTAINEROF_H 1

/*!
 * \file containerof.h
 * \brief Container-of macro for C structures.
 * This header file defines the `CONTAINER_OF` macro, which is used to obtain a
 * pointer to the containing structure from a pointer to one of its members.
 * It is useful in data structures where you need to access the parent structure
 * from a pointer to one of its members.
 */

#include <stddef.h>

/*!
 * \brief Casts member pointer to containing structure pointer.
 * \param ptr Pointer to member.
 * \param type Type of container structure.
 * \param member Name of container's member.
 *
 * Utilises GCC's \c offsetof built-in function to displace the member pointer
 * backwards by the difference between the container's address and the member's
 * address. Subtracting a size from a void pointer decrements the pointer by one
 * byte for each displacement unit.
 *
 * This macro is useful for obtaining a pointer to the containing structure from
 * a pointer to one of its members. It is commonly used in data structures where
 * you need to access the parent structure from a pointer to one of its members.
 * For example, if you have a linked list node and you want to access the
 * containing structure that holds the node, you can use this macro.
 *
 * It relies on void pointer arithmetic and the \c offsetof macro, which is part of
 * the C standard library. The \c offsetof macro is used to calculate the offset
 * of a member within a structure, allowing the macro to compute the address of
 * the containing structure from the address of a member.
 */
#ifdef __GNUC__
/*
 * This version of the macro uses a statement expression, which is a GNU C
 * extension. It also relies on void-pointer arithmetic, a useful technique in C
 * for implementing container-of functionality.
 */
#define CONTAINER_OF(ptr, type, member)            \
  ({                                               \
    void *void_ptr = (void *)(ptr);                \
    ((type *)(void_ptr - offsetof(type, member))); \
  })
#endif /* __GNUC__ */

/*!
 * \brief Alternative container-of macro without statement expression.
 * \param ptr Pointer to member.
 * \param type Type of container structure.
 * \param member Name of container's member.
 */
#define containerof(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

#endif /* CONTAINEROF_H */
