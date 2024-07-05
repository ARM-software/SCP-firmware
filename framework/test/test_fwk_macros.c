/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_test.h>

static void test_fwk_macros_array_size(void);
static void test_fwk_macros_align_next(void);
static void test_fwk_macros_align_previous(void);
static void test_fwk_macros_power_of_two_check(void);

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_macros_array_size),
    FWK_TEST_CASE(test_fwk_macros_align_next),
    FWK_TEST_CASE(test_fwk_macros_align_previous),
    FWK_TEST_CASE(test_fwk_macros_power_of_two_check)
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

static void test_fwk_macros_power_of_two_check(void)
{
    bool result;
    unsigned int value;
    struct power_of_two_test_data {
        unsigned long long value;
        bool verdict;
    } test_data[] = {
        { 0x00000, false }, { 0x00017, false },   { 0x0006C, false },
        { 0x0004D, false }, { 0x00017, false },   { 0x002aC, false },
        { 0x102aC, false }, { 0x00001, true },    { 0x00100, true },
        { 0x00004, true },  { 0x00080, true },    { 0x100000, true },
        { 0x400000, true }, { 0x80000000, true }, { 0xFFFFFFFF, false },
    };
    unsigned int test_count = sizeof(test_data) / sizeof(test_data[0]);

    /* Precedence tests */
    value = 3;
    result = FWK_IS_VALUE_POWER_OF_TWO(value & 1);
    assert(result == true);

    value = 3;
    result = FWK_IS_VALUE_POWER_OF_TWO(value & 2);
    assert(result == true);

    value = 5;
    result = FWK_IS_VALUE_POWER_OF_TWO(value & 1);
    assert(result == true);

    value = 5;
    result = FWK_IS_VALUE_POWER_OF_TWO(value & 4);
    assert(result == true);

    for (unsigned int i = 0; i < test_count; ++i) {
        result = FWK_IS_VALUE_POWER_OF_TWO(test_data[i].value);
        assert(result == test_data[i].verdict);
    }
}
