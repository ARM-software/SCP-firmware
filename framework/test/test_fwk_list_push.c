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
#include <string.h>

static struct fwk_slist slist;
static struct fwk_dlist dlist;

static struct fwk_slist_node snodes[2];
static struct fwk_dlist_node dnodes[2];

static void test_case_setup(void)
{
    fwk_list_init(&slist);
    fwk_list_init(&dlist);

    /* Remove node links before each test case */
    memset(snodes, 0, sizeof(snodes));
    memset(dnodes, 0, sizeof(dnodes));
}

static void test_slist_push_head_from_empty(void)
{
    fwk_list_push_head(&slist, &snodes[0]);

    assert(slist.head == &snodes[0]);
    assert(slist.tail == &snodes[0]);

    assert(snodes[0].next == (struct fwk_slist_node *)&slist);
}

static void test_dlist_push_head_from_empty(void)
{
    fwk_list_push_head(&dlist, &dnodes[0]);

    assert(dlist.head == &dnodes[0]);
    assert(dlist.tail == &dnodes[0]);

    assert(dnodes[0].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[0].prev == (struct fwk_dlist_node *)&dlist);
}

static void test_slist_push_back_from_empty(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);

    assert(slist.head == &snodes[0]);
    assert(slist.tail == &snodes[0]);

    assert(snodes[0].next == (struct fwk_slist_node *)&slist);
}

static void test_dlist_push_back_from_empty(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);

    assert(dlist.head == &dnodes[0]);
    assert(dlist.tail == &dnodes[0]);

    assert(dnodes[0].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[0].prev == (struct fwk_dlist_node *)&dlist);
}

static void test_slist_push_head(void)
{
    fwk_list_push_head(&slist, &snodes[0]);
    fwk_list_push_head(&slist, &snodes[1]);

    assert(slist.head == &snodes[1]);
    assert(slist.tail == &snodes[0]);

    assert(snodes[0].next == (struct fwk_slist_node *)&slist);
    assert(snodes[1].next == &snodes[0]);
}

static void test_dlist_push_head(void)
{
    fwk_list_push_head(&dlist, &dnodes[0]);
    fwk_list_push_head(&dlist, &dnodes[1]);

    assert(dlist.head == &dnodes[1]);
    assert(dlist.tail == &dnodes[0]);

    assert(dnodes[0].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[0].prev == &dnodes[1]);

    assert(dnodes[1].next == &dnodes[0]);
    assert(dnodes[1].prev == (struct fwk_dlist_node *)&dlist);
}

static void test_slist_push_back(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);

    assert(slist.head == &snodes[0]);
    assert(slist.tail == &snodes[1]);

    assert(snodes[0].next == &snodes[1]);
    assert(snodes[1].next == (struct fwk_slist_node *)&slist);
}

static void test_dlist_push_back(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);

    assert(dlist.head == &dnodes[0]);
    assert(dlist.tail == &dnodes[1]);

    assert(dnodes[0].next == &dnodes[1]);
    assert(dnodes[0].prev == (struct fwk_dlist_node *)&dlist);

    assert(dnodes[1].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[1].prev == &dnodes[0]);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_push_head_from_empty),
    FWK_TEST_CASE(test_dlist_push_head_from_empty),
    FWK_TEST_CASE(test_slist_push_back_from_empty),
    FWK_TEST_CASE(test_dlist_push_back_from_empty),
    FWK_TEST_CASE(test_slist_push_head),
    FWK_TEST_CASE(test_dlist_push_head),
    FWK_TEST_CASE(test_slist_push_back),
    FWK_TEST_CASE(test_dlist_push_back),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_push",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
