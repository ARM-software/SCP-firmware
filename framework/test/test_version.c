/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_macros.h>
#include <fwk_test.h>

static void test_version(void)
{
    assert(BUILD_VERSION_MAJOR == 2);
    assert(BUILD_VERSION_MINOR == 4);
    assert(BUILD_VERSION_PATCH == 0);
}

static void test_version_string(void)
{
    const char *result_string = "v2.4.0";

    assert(strcmp(BUILD_VERSION_STRING, result_string) == 0);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_version),
    FWK_TEST_CASE(test_version_string),
};

struct fwk_test_suite_desc test_suite = {
    .name = "version",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
