/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
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
 * \{
 */

/*!
 * \addtogroup GroupLinkedList Linked Lists
 * \{
 */

/*!
 * \brief marks used for marked slist.
 *
 * \internal
 * \note This structure used in case of needed to mark dlist
 */
struct fwk_dlist_marks {
    /*! Current numbers of dlist elements */
    unsigned int current_count;

    /*! Maximum marked elements in dlist */
    unsigned int max_count;
};

/*!
 * \brief Doubly-linked list.
 *
 * \internal
 * \note This structure can be safely used in the place of ::fwk_dlist,
 *      ::fwk_dlist_node, or ::fwk_dlist_node.
 */
struct fwk_dlist {
    /*! Pointer to the list head */
    struct fwk_dlist_node *head;

    /*! Pointer to the list tail */
    struct fwk_dlist_node *tail;

#ifdef FWK_MARKED_LIST_ENABLE
    /*! save the mark for maximum usage of dlist */
    struct fwk_dlist_marks marks;
#endif
};

/*!
 * \brief Doubly-linked list node.
 *
 * \internal
 * \note This structure can be safely used in the place of ::fwk_dlist_node.
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
void __fwk_dlist_push_head(struct fwk_dlist *list, struct fwk_dlist_node *new)
    FWK_LEAF FWK_NOTHROW FWK_NONNULL(1) FWK_NONNULL(2) FWK_READ_WRITE1(1)
        FWK_READ_WRITE1(2);

/*
 * Add a new node to the end of a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_push_tail(list, new) for the public interface.
 */
void __fwk_dlist_push_tail(struct fwk_dlist *list, struct fwk_dlist_node *new)
    FWK_LEAF FWK_NOTHROW FWK_NONNULL(1) FWK_NONNULL(2) FWK_READ_WRITE1(1)
        FWK_READ_WRITE1(2);

/*
 * Remove and return the head node from a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_pop_head(list) for the public interface.
 */
struct fwk_dlist_node *__fwk_dlist_pop_head(struct fwk_dlist *list) FWK_LEAF
    FWK_NOTHROW FWK_NONNULL(1) FWK_READ_WRITE1(1);

/*
 * Remove a node from a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_remove(list, node) for the public interface.
 */
void __fwk_dlist_remove(struct fwk_dlist *list, struct fwk_dlist_node *node)
    FWK_LEAF FWK_NOTHROW FWK_NONNULL(1) FWK_NONNULL(2) FWK_READ_ONLY1(1)
        FWK_READ_WRITE1(2);

/*
 * Insert a node into a doubly-linked list.
 *
 * For internal use only.
 * See fwk_list_insert(list, new, node) for the public interface.
 */
void __fwk_dlist_insert(
    struct fwk_dlist *list,
    struct fwk_dlist_node *restrict new,
    struct fwk_dlist_node *restrict node) FWK_LEAF FWK_NOTHROW FWK_NONNULL(1)
    FWK_NONNULL(2) FWK_READ_WRITE1(1) FWK_READ_WRITE1(2) FWK_READ_WRITE1(3);

#ifdef FWK_MARKED_LIST_ENABLE
/*
 * Return max size of dlist.
 *
 * For internal use only.
 * See __fwk_dlist_get_max(list) for the public interface.
 */
int __fwk_dlist_get_max(const struct fwk_dlist *list) FWK_LEAF FWK_NOTHROW
    FWK_NONNULL(1) FWK_READ_ONLY1(1);
#endif

/*!
 * @endcond
 */

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_DLIST_H */
