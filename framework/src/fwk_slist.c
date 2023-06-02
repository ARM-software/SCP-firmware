/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Intrusive circular singly-linked list.
 */

#include <fwk_assert.h>
#include <fwk_list.h>
#include <fwk_slist.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef FWK_MARKED_LIST_ENABLE
static void increment_slist_elements_count(struct fwk_slist_marks *slist_marks)
{
    slist_marks->current_count++;
    slist_marks->max_count =
        FWK_MAX(slist_marks->max_count, slist_marks->current_count);
}

static void decrement_slist_elements_count(struct fwk_slist_marks *slist_marks)
{
    if (slist_marks->current_count > 0) {
        --slist_marks->current_count;
    }
}
#endif

void __fwk_slist_init(struct fwk_slist *list)
{
    fwk_assert(list != NULL);

    list->head = (struct fwk_slist_node *)list;
    list->tail = (struct fwk_slist_node *)list;

#ifdef FWK_MARKED_LIST_ENABLE
    list->marks.current_count = 0;
    list->marks.max_count = 0;
#endif
}

struct fwk_slist_node *__fwk_slist_head(const struct fwk_slist *list)
{
    fwk_assert(list != NULL);

    if (__fwk_slist_is_empty(list)) {
        return NULL;
    }

    return list->head;
}

bool __fwk_slist_is_empty(const struct fwk_slist *list)
{
    bool is_empty;

    fwk_assert(list != NULL);

    is_empty = list->head == (struct fwk_slist_node *)list;

    if (is_empty) {
        fwk_assert(list->tail == list->head);
    }

    return is_empty;
}

void __fwk_slist_push_head(
    struct fwk_slist *list,
    struct fwk_slist_node *new)
{
    fwk_assert(list != NULL);
    fwk_assert(new != NULL);
    fwk_check(new->next == NULL);

    new->next = list->head;

    list->head = new;
    if (list->tail == (struct fwk_slist_node *)list) {
        list->tail = new;
    }

#ifdef FWK_MARKED_LIST_ENABLE
    increment_slist_elements_count(&list->marks);
#endif
}

void __fwk_slist_push_tail(
    struct fwk_slist *list,
    struct fwk_slist_node *new)
{
    fwk_assert(list != NULL);
    fwk_assert(new != NULL);
    fwk_check(new->next == NULL);

    new->next = (struct fwk_slist_node *)list;

    list->tail->next = new;
    list->tail = new;

#ifdef FWK_MARKED_LIST_ENABLE
    increment_slist_elements_count(&list->marks);
#endif
}

struct fwk_slist_node *__fwk_slist_pop_head(struct fwk_slist *list)
{
    struct fwk_slist_node *popped;

    fwk_assert(list != NULL);

    if (__fwk_slist_is_empty(list)) {
        return NULL;
    }

    popped = list->head;
    if (popped->next == (struct fwk_slist_node *)list) {
        list->tail = (struct fwk_slist_node *)list;
    }

    list->head = popped->next;

    popped->next = NULL;

#ifdef FWK_MARKED_LIST_ENABLE
    decrement_slist_elements_count(&list->marks);
#endif
    return popped;
}

struct fwk_slist_node *__fwk_slist_next(
    const struct fwk_slist *list,
    const struct fwk_slist_node *node)
{
    fwk_assert(list != NULL);
    fwk_assert(node != NULL);

    fwk_assert(__fwk_slist_contains(list, node));

    return (node->next == (struct fwk_slist_node *)list) ? NULL : node->next;
}

void __fwk_slist_remove(
    struct fwk_slist *list,
    struct fwk_slist_node *node)
{
    fwk_assert(list != NULL);
    fwk_assert(node != NULL);
    fwk_assert(node->next != NULL);

    struct fwk_slist_node *node_iter = (struct fwk_slist_node *)list;

    while (node_iter->next != (struct fwk_slist_node *)list) {
        if (node_iter->next == node) {
            node_iter->next = node->next;

            if (node->next == (struct fwk_slist_node *)list) {
                list->tail = (struct fwk_slist_node *)node_iter;
            }

            node->next = NULL;
#ifdef FWK_MARKED_LIST_ENABLE
            decrement_slist_elements_count(&list->marks);
#endif

            return;
        }
        node_iter = node_iter->next;
    }

    fwk_unexpected();
}

bool __fwk_slist_contains(
    const struct fwk_slist *list,
    const struct fwk_slist_node *node)
{
    const struct fwk_slist_node *node_iter;

    fwk_assert(list != NULL);
    fwk_assert(node != NULL);
    fwk_assert(node->next != NULL);

    node_iter = (struct fwk_slist_node *)list;

    while (node_iter->next != (struct fwk_slist_node *)list) {
        if (node_iter->next == node) {
            return true;
        }

        node_iter = node_iter->next;
    }

    return false;
}

#ifdef FWK_MARKED_LIST_ENABLE
int __fwk_slist_get_max(const struct fwk_slist *list)
{
    return list->marks.max_count;
}
#endif

static_assert(offsetof(struct fwk_slist, head) ==
    offsetof(struct fwk_slist_node, next),
    "fwk_slist::head not aligned with fwk_slist_node::next");
