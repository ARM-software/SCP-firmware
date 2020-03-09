/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_test.h>

#include <string.h>

static struct fwk_dlist list;
static struct fwk_dlist_node node[3];

static void setup(void)
{
    fwk_list_init(&list);

    /* Remove node links before each test case */
    memset(node, 0, sizeof(node));
}

static void test_list_insert(void)
{
    fwk_list_push_tail(&list, &node[0]);
    fwk_list_push_tail(&list, &node[2]);

    fwk_list_insert(&list, &node[1], &node[2]);

    assert(node[0].next == &node[1]);
    assert(node[1].next == &node[2]);
    assert(node[2].next == (struct fwk_dlist_node *)&list);

    assert(node[2].prev == &node[1]);
    assert(node[1].prev == &node[0]);
    assert(node[0].prev == (struct fwk_dlist_node *)&list);

    assert(list.head == &node[0]);
    assert(list.tail == &node[2]);
}

static void test_list_insert_empty(void)
{
    fwk_list_insert(&list, &node[0], NULL);

    assert(list.head == &node[0]);
    assert(list.tail == &node[0]);

    assert(node[0].next == (struct fwk_dlist_node *)&list);
    assert(node[0].prev == (struct fwk_dlist_node *)&list);
}

static void test_list_insert_tail_implicit(void)
{
    fwk_list_insert(&list, &node[0], NULL);

    assert(list.head == &node[0]);
    assert(list.tail == &node[0]);

    fwk_list_insert(&list, &node[1], NULL);

    assert(list.head == &node[0]);
    assert(list.tail == &node[1]);
}

static const struct fwk_test_case_desc tests[] = {
    FWK_TEST_CASE(test_list_insert),
    FWK_TEST_CASE(test_list_insert_empty),
    FWK_TEST_CASE(test_list_insert_tail_implicit),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_insert",
    .test_case_setup = setup,
    .test_case_table = tests,
    .test_case_count = FWK_ARRAY_SIZE(tests),
};
