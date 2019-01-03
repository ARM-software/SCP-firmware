/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Intrusive circular doubly-linked list.
 */

#ifndef FWK_DLIST_H
#define FWK_DLIST_H

#include <fwk_slist.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \addtogroup GroupLinkedList Linked Lists
 * @{
 */

/*!
 * \brief Doubly-linked list.
 *
 * \internal
 * \note This structure can be safely used in the place of \ref fwk_slist,
 *      \ref fwk_slist_node, or \ref fwk_dlist_node.
 */
struct fwk_dlist {
    /*! Pointer to the list head */
    struct fwk_dlist_node *head;

    /*! Pointer to the list tail */
    struct fwk_dlist_node *tail;
};

/*!
 * \brief Doubly-linked list node.
 *
 * \internal
 * \note This structure can be safely used in the place of \ref fwk_slist_node.
 */
struct fwk_dlist_node {
    /*! Pointer to the next node in the list */
    struct fwk_dlist_node *next;

    /*! Pointer to the previous node in the list */
    struct fwk_dlist_node *prev;
};

/*!
 * @cond
 */

/*
 * Add a new node to the head of a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_push_head(list, new) for the public interface.
 */
void __fwk_dlist_push_head(
    struct fwk_dlist *list,
    struct fwk_dlist_node *new);

/*
 * Add a new node to the end of a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_push_tail(list, new) for the public interface.
 */
void __fwk_dlist_push_tail(
    struct fwk_dlist *list,
    struct fwk_dlist_node *new);

/*
 * Remove and return the head node from a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_pop_head(list) for the public interface.
 */
struct fwk_dlist_node *__fwk_dlist_pop_head(struct fwk_dlist *list);

/*
 * Remove a node from a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_remove(list, node) for the public interface.
 */
void __fwk_dlist_remove(
    struct fwk_dlist *list,
    struct fwk_dlist_node *node);

/*
 * Insert a node into a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_insert(list, new, node) for the public interface.
 */
void __fwk_dlist_insert(
    struct fwk_dlist *list,
    struct fwk_dlist_node *restrict new,
    struct fwk_dlist_node *restrict node);

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_DLIST_H */
