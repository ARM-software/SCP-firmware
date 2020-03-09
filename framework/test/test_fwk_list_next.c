/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_test.h>

#include <stddef.h>

static struct fwk_slist slist;
static struct fwk_dlist dlist;

static struct fwk_slist_node snodes[2];
static struct fwk_dlist_node dnodes[2];

static void test_case_setup(void)
{
    fwk_list_init(&slist);
    fwk_list_init(&dlist);
}

static void test_slist_next(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);

    assert(fwk_list_next(&slist, &snodes[0]) == &snodes[1]);
    assert(fwk_list_next(&slist, &snodes[1]) == NULL);
}

static void test_dlist_next(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);

    assert(fwk_list_next(&dlist, &dnodes[0]) == &dnodes[1]);
    assert(fwk_list_next(&dlist, &dnodes[1]) == NULL);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_next),
    FWK_TEST_CASE(test_dlist_next),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_next",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
