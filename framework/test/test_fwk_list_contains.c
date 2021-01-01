/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_test.h>

static struct fwk_dlist list;
static struct fwk_dlist_node node[2];

static void setup(void)
{
    fwk_list_init(&list);
}

static void test_list_contains(void)
{
    node[0].next = node[0].prev = (struct fwk_dlist_node *)0xdead;
    node[1].next = node[1].prev = (struct fwk_dlist_node *)0xbeef;
    assert(fwk_list_contains(&list, &node[0]) == false);
    assert(fwk_list_contains(&list, &node[1]) == false);

    node[0].next = node[0].prev = NULL;
    fwk_list_push_tail(&list, &node[0]);
    assert(fwk_list_contains(&list, &node[0]) == true);
    assert(fwk_list_contains(&list, &node[1]) == false);

    node[1].next = node[1].prev = NULL;
    fwk_list_push_tail(&list, &node[1]);
    assert(fwk_list_contains(&list, &node[0]) == true);
    assert(fwk_list_contains(&list, &node[1]) == true);
}

static const struct fwk_test_case_desc tests[] = {
    FWK_TEST_CASE(test_list_contains),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_contains",
    .test_case_setup = setup,
    .test_case_table = tests,
    .test_case_count = FWK_ARRAY_SIZE(tests),
};
