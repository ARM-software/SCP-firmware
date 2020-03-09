/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Intrusive circular singly and doubly-linked lists.
 */

#ifndef FWK_LIST_H
#define FWK_LIST_H

#include <fwk_dlist.h>
#include <fwk_slist.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \addtogroup GroupLinkedList Linked Lists
 * @{
 */

/*!
 * \brief Get the container in which a linked list node is stored.
 *
 * \param node Pointer to the linked list node to get the container of.
 * \param type Type of the container.
 * \param member Name of the linked list node member.
 *
 * \return Pointer to the container.
 */
#define FWK_LIST_GET(node, type, member) \
    ((type *)(((uintptr_t)node) - offsetof(type, member)))

/*!
 * \brief Initialize a linked list.
 *
 * \param[out] list List to initialize. Must not be \c NULL.
 *
 * \return None.
 */
#define fwk_list_init(list) \
    _Generic((list), \
        struct fwk_slist * : __fwk_slist_init, \
        struct fwk_dlist * : __fwk_slist_init \
    )((struct fwk_slist *)list)

/*!
 * \brief Retrieve the node at the head of a linked list.
 *
 * \param list Pointer to the list to retrieve the head of. Must not be \c NULL.
 *
 * \retval NULL The list is empty.
 * \return Pointer to the node at the head of the linked list.
 */
#define fwk_list_head(list) \
    ((void *)_Generic((list), \
        const struct fwk_slist * : __fwk_slist_head, \
        const struct fwk_dlist * : __fwk_slist_head, \
        struct fwk_slist * : __fwk_slist_head, \
        struct fwk_dlist * : __fwk_slist_head \
    )((const struct fwk_slist *)list))

/*!
 * \brief Test whether a linked list is empty or not.
 *
 * \param list Pointer to the list to test. Must not be \c NULL.
 *
 * \retval true The linked list is empty.
 * \retval false The linked list is not empty.
 */
#define fwk_list_is_empty(list) \
    _Generic((list), \
        const struct fwk_slist * : __fwk_slist_is_empty, \
        const struct fwk_dlist * : __fwk_slist_is_empty, \
        struct fwk_slist * : __fwk_slist_is_empty, \
        struct fwk_dlist * : __fwk_slist_is_empty \
    )((const struct fwk_slist *)list)

/*!
 * \brief Add a new node to the head of a linked list.
 *
 * \param list Pointer to the list to add to. Must not be \c NULL.
 * \param new Pointer to the node to add. Must not be \c NULL. In debug mode,
 *      the node links must be \c NULL as they are checked to ensure the node is
 *      not already in use.
 *
 * \return None.
 */
#define fwk_list_push_head(list, new) \
    _Generic((list), \
        struct fwk_slist * : __fwk_slist_push_head, \
        struct fwk_dlist * : __fwk_dlist_push_head \
    )(list, new)

/*!
 * \brief Add a new node to the end of a linked list.
 *
 * \param list Pointer to the list to add to. Must not be \c NULL.
 * \param new Pointer to the node to add. Must not be \c NULL. In debug mode,
 *      the node links must be \c NULL as they are checked to ensure the node is
 *      not already in use.
 *
 * \return None.
 */
#define fwk_list_push_tail(list, new) \
    _Generic((list), \
        struct fwk_slist * : __fwk_slist_push_tail, \
        struct fwk_dlist * : __fwk_dlist_push_tail \
    )(list, new)

/*!
 * \brief Remove and return the head node from a linked list.
 *
 * \param list Pointer to the list to remove from. Must not be \c NULL.
 *
 * \retval NULL The list was empty.
 * \return The linked list node that was removed. In debug mode, the node links
 *      are set to \c NULL to ensure the node no longer references the list it
 *      has been removed from.
 */
#define fwk_list_pop_head(list) \
    _Generic((list), \
        struct fwk_slist * : __fwk_slist_pop_head, \
        struct fwk_dlist * : __fwk_dlist_pop_head \
    )(list)

/*!
 * \brief Get the next node of a linked list.
 *
 * \param list Pointer to the list to get the node from. Must not be \c NULL.
 * \param node Pointer to the node to get the next one from. Must not be \c
 *      NULL.
 *
 * \retval NULL 'node' was the last node of the list.
 * \return The pointer to the next node.
 */
#define fwk_list_next(list, node) \
    ((void *) _Generic((list), \
        const struct fwk_slist * : __fwk_slist_next, \
        const struct fwk_dlist * : __fwk_slist_next, \
        struct fwk_slist * : __fwk_slist_next, \
        struct fwk_dlist * : __fwk_slist_next \
    )((const struct fwk_slist *)list, (const struct fwk_slist_node *)node))

/*!
 * \brief Remove a node from a linked list.
 *
 * \details In debug mode, the node being removed has its links set to \c NULL
 * to ensure the node no longer references the list it has been removed from.
 *
 * \param list Pointer to the list to remove the node from. Must not be \c NULL.
 * \param node Pointer to the node to remove. Must not be \c NULL. The node must
 *      be in the list.
 *
 * \return None.
 */
#define fwk_list_remove(list, node) \
    _Generic((list), \
        struct fwk_slist * : __fwk_slist_remove, \
        struct fwk_dlist * : __fwk_dlist_remove \
    )(list, node)

/*!
 * \brief Insert a node into a linked list.
 *
 * \param list Pointer to the list. Must not be \c NULL.
 * \param new Pointer to the node to insert. Must not be \c NULL. In debug mode,
 *      the node links must be \c NULL as they are checked to ensure the node is
 *      not already in use.
 * \param node Pointer to the node that \p new will be inserted before.  If this
 *      is \c NULL then the new node will be inserted at the tail of the list.
 *
 * \return None.
 */
#define fwk_list_insert(list, new, node) \
    _Generic((list), \
        struct fwk_dlist * : __fwk_dlist_insert \
    )(list, new, node)

/*!
 * \brief Check if a node is in a list.
 *
 * \param list Pointer to the list. Must not be \c NULL.
 * \param node Pointer to the node. Must not be \c NULL.
 *
 * \retval true \p node is in \p list.
 * \retval false \p node is not in \p list.
 */
#define fwk_list_contains(list, node) \
    _Generic((list), \
        const struct fwk_slist * : __fwk_slist_contains, \
        const struct fwk_dlist * : __fwk_slist_contains, \
        struct fwk_slist * : __fwk_slist_contains, \
        struct fwk_dlist * : __fwk_slist_contains \
    )((const struct fwk_slist *)list, (const struct fwk_slist_node *)node)

/*!
 * \brief Iterate over all nodes in a list.
 *
 * \param list Pointer to the list. Must not be \c NULL.
 * \param node Pointer to the node. Must not be \c NULL.
 * \param type Type of the container structure that contains fwk_slist node.
 * \param member The name of the node element in the struct.
 * \param elem Pointer to the struct object to use inside the loop
 */
#define FWK_LIST_FOR_EACH(list, node, type, member, elem) \
    for (node = fwk_list_head(list), \
        elem = FWK_LIST_GET(node, type, member); \
        node != NULL; \
        node = fwk_list_next(list, node), \
        elem = FWK_LIST_GET(node, type, member))

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_LIST_H */
