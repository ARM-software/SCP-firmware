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

struct container {
    unsigned int value;

    union {
        struct fwk_slist_node slink;
        struct fwk_dlist_node dlink;
    };
};

static void test_slist_get(void)
{
    struct container container = { 42 };
    struct fwk_slist_node *node = &container.slink;

    assert(FWK_LIST_GET(node, struct container, slink)->value == 42);
}

static void test_dlist_get(void)
{
    struct container container = { 42 };
    struct fwk_dlist_node *node = &container.dlink;

    assert(FWK_LIST_GET(node, struct container, dlink)->value == 42);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_get),
    FWK_TEST_CASE(test_dlist_get),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_get",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
