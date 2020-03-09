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

#include <stddef.h>

static struct fwk_slist slist;
static struct fwk_dlist dlist;

static struct fwk_slist_node snode;
static struct fwk_dlist_node dnode;

static void test_case_setup(void)
{
    fwk_list_init(&slist);
    fwk_list_init(&dlist);
}

static void test_slist_empty(void)
{
    assert(fwk_list_is_empty(&slist));
}

static void test_dlist_empty(void)
{
    assert(fwk_list_is_empty(&dlist));
}

static void test_slist_not_empty(void)
{
    fwk_list_push_tail(&slist, &snode);

    assert(!fwk_list_is_empty(&slist));
}

static void test_dlist_not_empty(void)
{
    fwk_list_push_tail(&dlist, &dnode);

    assert(!fwk_list_is_empty(&dlist));
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_empty),
    FWK_TEST_CASE(test_dlist_empty),
    FWK_TEST_CASE(test_slist_not_empty),
    FWK_TEST_CASE(test_dlist_not_empty),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_empty",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
