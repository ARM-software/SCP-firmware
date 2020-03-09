/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_math.h>
#include <fwk_test.h>

#include <limits.h>
#include <stdint.h>

static void test_fwk_math_pow2_uc(void)
{
    unsigned char exp = (sizeof(exp) * CHAR_BIT) - 1;

    assert((fwk_math_pow2(exp) - 1) == SCHAR_MAX);
}

static void test_fwk_math_pow2_us(void)
{
    unsigned short exp = (sizeof(exp) * CHAR_BIT) - 1;

    assert((fwk_math_pow2(exp) - 1) == SHRT_MAX);
}

static void test_fwk_math_pow2_ui(void)
{
    unsigned int exp = (sizeof(exp) * CHAR_BIT) - 1;

    assert((fwk_math_pow2(exp) - 1) == INT_MAX);
}

static void test_fwk_math_pow2_ul(void)
{
    unsigned long exp = (sizeof(exp) * CHAR_BIT) - 1;

    assert((fwk_math_pow2(exp) - 1) == LONG_MAX);
}

static void test_fwk_math_pow2_ull(void)
{
    unsigned long long exp = (sizeof(exp) * CHAR_BIT) - 1;

    assert((fwk_math_pow2(exp) - 1) == LLONG_MAX);
}

static void test_fwk_math_clz_ui(void)
{
    unsigned int num = ~0u >> 5;

    assert(fwk_math_clz(num) == 5);
}

static void test_fwk_math_clz_ul(void)
{
    unsigned long num = ~0ul >> 5;

    assert(fwk_math_clz(num) == 5);
}

static void test_fwk_math_clz_ull(void)
{
    unsigned long long num = ~0ull >> 5;

    assert(fwk_math_clz(num) == 5);
}

static void test_fwk_math_log2_ui(void)
{
    unsigned int num = UINT_MAX;
    unsigned int expected = (sizeof(num) * CHAR_BIT) - 1;

    assert(fwk_math_log2(num) == expected);
}

static void test_fwk_math_log2_ul(void)
{
    unsigned long num = ULONG_MAX;
    unsigned long expected = (sizeof(num) * CHAR_BIT) - 1;

    assert(fwk_math_log2(num) == expected);
}

static void test_fwk_math_log2_ull(void)
{
    unsigned long long num = ULLONG_MAX;
    unsigned long long expected = (sizeof(num) * CHAR_BIT) - 1;

    assert(fwk_math_log2(num) == expected);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_math_pow2_uc),
    FWK_TEST_CASE(test_fwk_math_pow2_us),
    FWK_TEST_CASE(test_fwk_math_pow2_ui),
    FWK_TEST_CASE(test_fwk_math_pow2_ul),
    FWK_TEST_CASE(test_fwk_math_pow2_ull),
    FWK_TEST_CASE(test_fwk_math_clz_ui),
    FWK_TEST_CASE(test_fwk_math_clz_ul),
    FWK_TEST_CASE(test_fwk_math_clz_ull),
    FWK_TEST_CASE(test_fwk_math_log2_ui),
    FWK_TEST_CASE(test_fwk_math_log2_ul),
    FWK_TEST_CASE(test_fwk_math_log2_ull),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_math",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
