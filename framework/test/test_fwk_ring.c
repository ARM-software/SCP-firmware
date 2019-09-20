/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_macros.h>
#include <fwk_ring.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <assert.h>
#include <stddef.h>

static struct fwk_ring ring;

static int test_suite_setup(void)
{
    static char ring_storage[4] = { 0 };

    fwk_ring_init(&ring, ring_storage, sizeof(ring_storage));

    return FWK_SUCCESS;
}

static void test_case_setup(void)
{
    fwk_ring_clear(&ring);
}

static void test_fwk_ring_pop_empty(void)
{
    char data_out[4] = { 127, 127, 127, 127 };

    size_t data_length = fwk_ring_pop(&ring, data_out, 4);
    assert(data_length == 0);

    assert(fwk_ring_get_length(&ring) == 0);
    assert(fwk_ring_is_empty(&ring) == true);
    assert(fwk_ring_is_full(&ring) == false);

    assert(data_out[0] == 127);
    assert(data_out[1] == 127);
    assert(data_out[2] == 127);
    assert(data_out[3] == 127);
}

static void test_fwk_ring_pop_linear(void)
{
    size_t data_length;

    const char data_in[4] = { 0, 1, 2, 3 };
    char data_out[4] = { 127, 127, 127, 127 };

    fwk_ring_push(&ring, data_in, 4);

    data_length = fwk_ring_pop(&ring, data_out, 4);
    assert(data_length == 4);

    assert(fwk_ring_get_length(&ring) == 0);
    assert(fwk_ring_is_empty(&ring) == true);
    assert(fwk_ring_is_full(&ring) == false);

    assert(data_out[0] == 0);
    assert(data_out[1] == 1);
    assert(data_out[2] == 2);
    assert(data_out[3] == 3);
}

static void test_fwk_ring_pop_fragmented(void)
{
    size_t data_length;

    const char data_in[6] = { 0, 1, 2, 3, 4, 5 };
    char data_out[4] = { 127, 127, 127, 127 };

    fwk_ring_push(&ring, &data_in[0], 4);
    fwk_ring_push(&ring, &data_in[4], 2);

    data_length = fwk_ring_pop(&ring, data_out, 4);
    assert(data_length == 4);

    assert(fwk_ring_get_length(&ring) == 0);
    assert(fwk_ring_is_empty(&ring) == true);
    assert(fwk_ring_is_full(&ring) == false);

    assert(data_out[0] == 2);
    assert(data_out[1] == 3);
    assert(data_out[2] == 4);
    assert(data_out[3] == 5);
}

static void test_fwk_ring_pop_exceeds_length(void)
{
    size_t data_length;

    const char data_in[2] = { 0, 1 };
    char data_out[4] = { 127, 127, 127, 127 };

    fwk_ring_push(&ring, data_in, 2);

    data_length = fwk_ring_pop(&ring, data_out, 4);
    assert(data_length == 2);

    assert(fwk_ring_get_length(&ring) == 0);
    assert(fwk_ring_is_empty(&ring) == true);
    assert(fwk_ring_is_full(&ring) == false);

    assert(data_out[0] == 0);
    assert(data_out[1] == 1);
    assert(data_out[2] == 127);
    assert(data_out[3] == 127);
}

static void test_fwk_ring_pop_partial_length(void)
{
    size_t data_length;

    const char data_in[4] = { 0, 1, 2, 3 };
    char data_out[4] = { 127, 127, 127, 127 };

    fwk_ring_push(&ring, data_in, 4);

    data_length = fwk_ring_pop(&ring, data_out, 3);
    assert(data_length == 3);

    assert(fwk_ring_get_length(&ring) == 1);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == false);

    assert(data_out[0] == 0);
    assert(data_out[1] == 1);
    assert(data_out[2] == 2);
    assert(data_out[3] == 127);
}

static void test_fwk_ring_pop_zero_length(void)
{
    size_t data_length;

    const char data_in[4] = { 0, 1, 2, 3 };
    char data_out[4] = { 127, 127, 127, 127 };

    fwk_ring_push(&ring, data_in, 4);

    data_length = fwk_ring_pop(&ring, data_out, 0);
    assert(data_length == 0);

    assert(fwk_ring_get_length(&ring) == 4);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == true);

    assert(data_out[0] == 127);
    assert(data_out[1] == 127);
    assert(data_out[2] == 127);
    assert(data_out[3] == 127);
}

static void test_fwk_ring_pop_null(void)
{
    size_t data_length;

    const char data_in[4] = { 0, 1, 2, 3 };
    char data_out[4] = { 127, 127, 127, 127 };

    fwk_ring_push(&ring, data_in, 4);

    data_length = fwk_ring_pop(&ring, NULL, 3);
    assert(data_length == 3);

    assert(fwk_ring_get_length(&ring) == 1);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == false);

    data_length = fwk_ring_peek(&ring, data_out, 1);
    assert(data_length == 1);

    assert(data_out[0] == 3);
    assert(data_out[1] == 127);
    assert(data_out[2] == 127);
    assert(data_out[3] == 127);
}

static void test_fwk_ring_push_exceeds_capacity(void)
{
    size_t data_length;

    const char data_in[6] = { 0, 1, 2, 3, 4, 5 };
    char data_out[4] = { 127, 127, 127, 127 };

    data_length = fwk_ring_push(&ring, data_in, 6);
    assert(data_length == 4);

    assert(fwk_ring_get_length(&ring) == 4);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == true);

    fwk_ring_pop(&ring, data_out, 4);

    assert(data_out[0] == 2);
    assert(data_out[1] == 3);
    assert(data_out[2] == 4);
    assert(data_out[3] == 5);
}

static void test_fwk_ring_push_multiple_linear(void)
{
    size_t data_length;

    const char data_in[4] = { 0, 1, 2, 3 };
    char data_out[4] = { 127, 127, 127, 127 };

    data_length = fwk_ring_push(&ring, &data_in[0], 2);
    assert(data_length == 2);

    assert(fwk_ring_get_length(&ring) == 2);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == false);

    data_length = fwk_ring_push(&ring, &data_in[2], 2);
    assert(data_length == 2);

    assert(fwk_ring_get_length(&ring) == 4);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == true);

    fwk_ring_pop(&ring, data_out, 4);

    assert(data_out[0] == 0);
    assert(data_out[1] == 1);
    assert(data_out[2] == 2);
    assert(data_out[3] == 3);
}

static void test_fwk_ring_push_multiple_fragmented(void)
{
    size_t data_length;

    const char data_in[6] = { 0, 1, 2, 3, 4, 5 };
    char data_out[4] = { 127, 127, 127, 127 };

    data_length = fwk_ring_push(&ring, &data_in[0], 3);
    assert(data_length == 3);

    assert(fwk_ring_get_length(&ring) == 3);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == false);

    data_length = fwk_ring_push(&ring, &data_in[3], 3);
    assert(data_length == 3);

    assert(fwk_ring_get_length(&ring) == 4);
    assert(fwk_ring_is_empty(&ring) == false);
    assert(fwk_ring_is_full(&ring) == true);

    fwk_ring_pop(&ring, data_out, 4);

    assert(data_out[0] == 2);
    assert(data_out[1] == 3);
    assert(data_out[2] == 4);
    assert(data_out[3] == 5);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_ring_pop_empty),
    FWK_TEST_CASE(test_fwk_ring_pop_linear),
    FWK_TEST_CASE(test_fwk_ring_pop_fragmented),
    FWK_TEST_CASE(test_fwk_ring_pop_exceeds_length),
    FWK_TEST_CASE(test_fwk_ring_pop_partial_length),
    FWK_TEST_CASE(test_fwk_ring_pop_zero_length),
    FWK_TEST_CASE(test_fwk_ring_pop_null),
    FWK_TEST_CASE(test_fwk_ring_push_exceeds_capacity),
    FWK_TEST_CASE(test_fwk_ring_push_multiple_linear),
    FWK_TEST_CASE(test_fwk_ring_push_multiple_fragmented),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_ring",

    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,

    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
