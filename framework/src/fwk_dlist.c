/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Intrusive circular doubly-linked list.
 */

#include <fwk_assert.h>
#include <fwk_dlist.h>
#include <fwk_slist.h>

#include <stddef.h>

void __fwk_dlist_push_head(
    struct fwk_dlist *list,
    struct fwk_dlist_node *new)
{
    assert(list != NULL);
    assert(new != NULL);
    fwk_expect(new->prev == NULL);

    new->prev = (struct fwk_dlist_node *)list;
    list->head->prev = new;

    __fwk_slist_push_head(
        (struct fwk_slist *)list,
        (struct fwk_slist_node *)new);
}

void __fwk_dlist_push_tail(
    struct fwk_dlist *list,
    struct fwk_dlist_node *new)
{
    assert(list != NULL);
    assert(new != NULL);
    fwk_expect(new->prev == NULL);

    new->prev = list->tail;

    __fwk_slist_push_tail(
        (struct fwk_slist *)list,
        (struct fwk_slist_node *)new);
}

struct fwk_dlist_node *__fwk_dlist_pop_head(struct fwk_dlist *list)
{
    struct fwk_dlist_node *popped;

    assert(list != NULL);

    popped = (struct fwk_dlist_node *)__fwk_slist_pop_head(
        (struct fwk_slist *)list);

    list->head->prev = (struct fwk_dlist_node *)list;

    if (popped != NULL)
        popped->prev = NULL;

    return popped;
}

void __fwk_dlist_remove(
    struct fwk_dlist *list,
    struct fwk_dlist_node *node)
{
    assert(list != NULL);
    assert(node != NULL);
    assert(node != (struct fwk_dlist_node *)list);
    assert(node->prev != NULL);
    assert(node->next != NULL);

    assert(__fwk_slist_contains(
        (struct fwk_slist *)list,
        (struct fwk_slist_node *)node));

    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->prev = NULL;
    node->next = NULL;
}

void __fwk_dlist_insert(
    struct fwk_dlist *list,
    struct fwk_dlist_node *restrict new,
    struct fwk_dlist_node *restrict node)
{
    assert(list != NULL);
    assert(new != NULL);
    assert(new != node);
    fwk_expect(new->next == NULL);
    fwk_expect(new->prev == NULL);

    if (node == NULL) {
        __fwk_dlist_push_tail(list, new);

        return;
    }

    assert(node->prev != NULL);
    assert(node->next != NULL);

    assert(__fwk_slist_contains(
        (struct fwk_slist *)list,
        (struct fwk_slist_node *)node));

    node->prev->next = new;
    new->prev = node->prev;
    new->next = node;
    node->prev = new;
}

static_assert(offsetof(struct fwk_dlist, head) ==
    offsetof(struct fwk_slist, head),
    "fwk_dlist::head not aligned with fwk_slist::head");

static_assert(offsetof(struct fwk_dlist, tail) ==
    offsetof(struct fwk_slist, tail),
    "fwk_dlist::tail not aligned with fwk_slist::tail");

static_assert(offsetof(struct fwk_dlist, head) ==
    offsetof(struct fwk_slist_node, next),
    "fwk_dlist::head not aligned with fwk_slist_node::next");

static_assert(offsetof(struct fwk_dlist, head) ==
    offsetof(struct fwk_dlist_node, next),
    "fwk_dlist::head not aligned with fwk_dlist_node::next");

static_assert(offsetof(struct fwk_dlist, tail) ==
    offsetof(struct fwk_dlist_node, prev),
    "fwk_dlist::tail not aligned with fwk_dlist_node::prev");
