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

static struct fwk_slist_node snodes[3];
static struct fwk_dlist_node dnodes[3];

static void test_case_setup(void)
{
    fwk_list_init(&slist);
    fwk_list_init(&dlist);
}

static void test_slist_pop_head_on_empty(void)
{
    assert(fwk_list_pop_head(&slist) == NULL);

    assert(slist.head == (struct fwk_slist_node *)&slist);
    assert(slist.tail == (struct fwk_slist_node *)&slist);
}

static void test_dlist_pop_head_on_empty(void)
{
    assert(fwk_list_pop_head(&dlist) == NULL);

    assert(dlist.head == (struct fwk_dlist_node *)&dlist);
    assert(dlist.tail == (struct fwk_dlist_node *)&dlist);
}

static void test_slist_pop_head_on_one(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);

    assert(fwk_list_pop_head(&slist) == &snodes[0]);

    assert(slist.head == (struct fwk_slist_node *)&slist);
    assert(slist.tail == (struct fwk_slist_node *)&slist);
}

static void test_dlist_pop_head_on_one(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);

    assert(fwk_list_pop_head(&dlist) == &dnodes[0]);

    assert(dlist.head == (struct fwk_dlist_node *)&dlist);
    assert(dlist.tail == (struct fwk_dlist_node *)&dlist);
}

static void test_slist_pop_head_on_two(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);

    assert(fwk_list_pop_head(&slist) == &snodes[0]);

    assert(slist.head == &snodes[1]);
    assert(slist.tail == &snodes[1]);

    assert(snodes[1].next == (struct fwk_slist_node *)&slist);
}

static void test_dlist_pop_head_on_two(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);

    assert(fwk_list_pop_head(&dlist) == &dnodes[0]);

    assert(dlist.head == &dnodes[1]);
    assert(dlist.tail == &dnodes[1]);

    assert(dnodes[1].prev == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[1].next == (struct fwk_dlist_node *)&dlist);
}

static void test_slist_pop_head_on_many(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);
    fwk_list_push_tail(&slist, &snodes[2]);

    assert(fwk_list_pop_head(&slist) == &snodes[0]);

    assert(slist.head == &snodes[1]);
    assert(slist.tail == &snodes[2]);

    assert(snodes[1].next == &snodes[2]);
    assert(snodes[2].next == (struct fwk_slist_node *)&slist);
}

static void test_dlist_pop_head_on_many(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);
    fwk_list_push_tail(&dlist, &dnodes[2]);

    assert(fwk_list_pop_head(&dlist) == &dnodes[0]);

    assert(dlist.head == &dnodes[1]);
    assert(dlist.tail == &dnodes[2]);

    assert(dnodes[1].prev == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[1].next == &dnodes[2]);
    assert(dnodes[2].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[2].prev == &dnodes[1]);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_pop_head_on_empty),
    FWK_TEST_CASE(test_dlist_pop_head_on_empty),
    FWK_TEST_CASE(test_slist_pop_head_on_one),
    FWK_TEST_CASE(test_dlist_pop_head_on_one),
    FWK_TEST_CASE(test_slist_pop_head_on_two),
    FWK_TEST_CASE(test_dlist_pop_head_on_two),
    FWK_TEST_CASE(test_slist_pop_head_on_many),
    FWK_TEST_CASE(test_dlist_pop_head_on_many),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_pop",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
