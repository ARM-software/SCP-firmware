/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_test.h>

static void test_fwk_macros_array_size(void);
static void test_fwk_macros_align_next(void);
static void test_fwk_macros_align_previous(void);

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_macros_array_size),
    FWK_TEST_CASE(test_fwk_macros_align_next),
    FWK_TEST_CASE(test_fwk_macros_align_previous)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_macros",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};

static void test_fwk_macros_array_size(void)
{
    unsigned int element_no;
    unsigned int array_uint[5];
    char array_char[3];

    element_no = FWK_ARRAY_SIZE(array_uint);
    assert(element_no == 5);

    element_no = FWK_ARRAY_SIZE(array_char);
    assert(element_no == 3);
}

static void test_fwk_macros_align_next(void)
{
    unsigned int value;
    unsigned int interval;
    unsigned int result;

    value = 0;
    interval = 32;

    /* Precedence test */
    result = FWK_ALIGN_NEXT(value | 1, interval + 32);
    assert(result == 64);

    value = 0;
    interval = 32;
    result = FWK_ALIGN_NEXT(value, interval);
    assert(result == 0);

    value = 8;
    interval = 8;
    result = FWK_ALIGN_NEXT(value, interval);
    assert(result == 8);

    value = 9;
    interval = 8;
    result = FWK_ALIGN_NEXT(value, interval);
    assert(result == 16);
}

static void test_fwk_macros_align_previous(void)
{
    unsigned int value;
    unsigned int interval;
    unsigned int result;

    value = 65;
    interval = 32;

    /* Precedence test */
    result = FWK_ALIGN_PREVIOUS(value & 1, interval + 32);
    assert(result == 0);

    value = 0;
    interval = 32;
    result = FWK_ALIGN_PREVIOUS(value, interval);
    assert(result == 0);

    value = 8;
    interval = 8;
    result = FWK_ALIGN_PREVIOUS(value, interval);
    assert(result == 8);

    value = 9;
    interval = 8;
    result = FWK_ALIGN_PREVIOUS(value, interval);
    assert(result == 8);
}
