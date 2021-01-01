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

static void test_slist_init(void)
{
    struct fwk_slist list;

    fwk_list_init(&list);

    assert(list.head == (struct fwk_slist_node *)&list);
    assert(list.tail == (struct fwk_slist_node *)&list);
}

static void test_dlist_init(void)
{
    struct fwk_dlist list;

    fwk_list_init(&list);

    assert(list.head == (struct fwk_dlist_node *)&list);
    assert(list.tail == (struct fwk_dlist_node *)&list);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_init),
    FWK_TEST_CASE(test_dlist_init),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_init",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
