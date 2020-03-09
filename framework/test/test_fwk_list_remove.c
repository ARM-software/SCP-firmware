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
#include <string.h>

static struct fwk_slist slist;
static struct fwk_dlist dlist;

static struct fwk_slist_node snodes[3];
static struct fwk_dlist_node dnodes[3];

static void test_case_setup(void)
{
    fwk_list_init(&slist);
    fwk_list_init(&dlist);

    /* Remove node links before each test case */
    memset(dnodes, 0, sizeof(dnodes));
    memset(snodes, 0, sizeof(snodes));
}

static void test_slist_remove_on_one(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);

    fwk_list_remove(&slist, &snodes[0]);
    assert(slist.head == (struct fwk_slist_node *)&slist);
    assert(slist.tail == (struct fwk_slist_node *)&slist);
}

static void test_slist_remove_first_of_two(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);

    fwk_list_remove(&slist, &snodes[0]);
    assert(slist.head == &snodes[1]);
    assert(slist.tail == &snodes[1]);
    assert(snodes[1].next == (struct fwk_slist_node *)&slist);
}

static void test_slist_remove_second_of_two(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);

    fwk_list_remove(&slist, &snodes[1]);
    assert(slist.head == &snodes[0]);
    assert(slist.tail == &snodes[0]);
    assert(snodes[0].next == (struct fwk_slist_node *)&slist);
}

static void test_slist_remove_second_of_three(void)
{
    fwk_list_push_tail(&slist, &snodes[0]);
    fwk_list_push_tail(&slist, &snodes[1]);
    fwk_list_push_tail(&slist, &snodes[2]);

    fwk_list_remove(&slist, &snodes[1]);
    assert(slist.head == &snodes[0]);
    assert(slist.tail == &snodes[2]);
    assert(snodes[0].next == &snodes[2]);
    assert(snodes[2].next == (struct fwk_slist_node *)&slist);
}
static void test_dlist_remove_on_one(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);

    fwk_list_remove(&dlist, &dnodes[0]);
    assert(dlist.head == (struct fwk_dlist_node *)&dlist);
    assert(dlist.tail == (struct fwk_dlist_node *)&dlist);
}

static void test_dlist_remove_first_of_two(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);

    fwk_list_remove(&dlist, &dnodes[0]);
    assert(dlist.head == &dnodes[1]);
    assert(dlist.tail == &dnodes[1]);
    assert(dnodes[1].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[1].prev == (struct fwk_dlist_node *)&dlist);
}

static void test_dlist_remove_second_of_two(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);

    fwk_list_remove(&dlist, &dnodes[1]);
    assert(dlist.head == &dnodes[0]);
    assert(dlist.tail == &dnodes[0]);
    assert(dnodes[0].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[0].prev == (struct fwk_dlist_node *)&dlist);
}

static void test_dlist_remove_second_of_three(void)
{
    fwk_list_push_tail(&dlist, &dnodes[0]);
    fwk_list_push_tail(&dlist, &dnodes[1]);
    fwk_list_push_tail(&dlist, &dnodes[2]);

    fwk_list_remove(&dlist, &dnodes[1]);
    assert(dlist.head == &dnodes[0]);
    assert(dlist.tail == &dnodes[2]);
    assert(dnodes[0].next == &dnodes[2]);
    assert(dnodes[0].prev == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[2].next == (struct fwk_dlist_node *)&dlist);
    assert(dnodes[2].prev == &dnodes[0]);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_slist_remove_on_one),
    FWK_TEST_CASE(test_slist_remove_first_of_two),
    FWK_TEST_CASE(test_slist_remove_second_of_two),
    FWK_TEST_CASE(test_slist_remove_second_of_three),
    FWK_TEST_CASE(test_dlist_remove_on_one),
    FWK_TEST_CASE(test_dlist_remove_first_of_two),
    FWK_TEST_CASE(test_dlist_remove_second_of_two),
    FWK_TEST_CASE(test_dlist_remove_second_of_three),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_list_remove",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
