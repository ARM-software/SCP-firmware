/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>
#include <fwk_trace.h>

#include <assert.h>
#include <stddef.h>

#define TRACE_ENTRY_COUNT (3)

static fwk_trace_count_t current_trace_count = 0;
static fwk_trace_count_t expected_trace_entry_count = 0;
static fwk_trace_id_t expected_id = 0;

static fwk_trace_count_t get_trace_count(void)
{
    return current_trace_count;
}

void report_trace_entry(
    const char *filename,
    const char *func,
    const unsigned int line,
    const fwk_trace_id_t id,
    const fwk_trace_count_t trace_count,
    const char *msg)
{
    assert(expected_id == id);
    assert(expected_trace_entry_count == trace_count);
}

static const struct fwk_trace_driver default_driver = {
    .trace_entry_count = TRACE_ENTRY_COUNT,
    .get_trace_count = get_trace_count,
    .report_trace_entry = report_trace_entry,
};

static struct fwk_trace_driver driver;

struct fwk_trace_driver fmw_trace_driver(void)
{
    return driver;
}

static int test_suite_setup(void)
{
    return FWK_SUCCESS;
}

static void test_case_setup(void)
{
    driver = default_driver;
    fwk_trace_init();
}

static void test_fwk_trace_start_invalid_id(void)
{
    int status = FWK_TRACE_START(TRACE_ENTRY_COUNT);
    assert(status == FWK_E_PARAM);
    status = FWK_TRACE_START(TRACE_ENTRY_COUNT + 1);
    assert(status == FWK_E_PARAM);
    status = FWK_TRACE_START(-1);
    assert(status == FWK_E_PARAM);
}

static void test_fwk_trace_start_twice(void)
{
    int status = FWK_TRACE_START(0);
    int status2 = FWK_TRACE_START(0);
    assert(status == FWK_SUCCESS);
    assert(status2 == FWK_E_STATE);
}

static void test_fwk_trace_start_no_driver(void)
{
    driver.get_trace_count = NULL;
    fwk_trace_init();
    int status = FWK_TRACE_START(0);
    assert(status == FWK_E_DEVICE);
}

static void test_fwk_trace_finish_invalid_id(void)
{
    int status = FWK_TRACE_FINISH(TRACE_ENTRY_COUNT, "");
    assert(status == FWK_E_PARAM);
    status = FWK_TRACE_FINISH(TRACE_ENTRY_COUNT + 1, "");
    assert(status == FWK_E_PARAM);
    status = FWK_TRACE_FINISH(-1, "");
    assert(status == FWK_E_PARAM);
}

static void test_fwk_trace_finish_with_no_start(void)
{
    int status = FWK_TRACE_FINISH(0, "");
    assert(status == FWK_E_STATE);
}

static void test_fwk_trace_normal_usage(void)
{
    current_trace_count = 1000;
    int status = FWK_TRACE_START(0);
    assert(status == FWK_SUCCESS);
    current_trace_count = 3000;
    expected_id = 0;
    expected_trace_entry_count = 2000;
    status = FWK_TRACE_FINISH(0, "");
    assert(status == FWK_SUCCESS);
}

static void test_fwk_trace_full_nesting(void)
{
    /*
     *  ----(1000)------(2000)----(3000)-----(4000)---------->
     *  ----[s:0]-------[s:1]-----[f:1]------[f:0]----------->
     */
    fwk_trace_count_t trace_counts[] = { 1000, 2000, 3000, 4000 };
    current_trace_count = trace_counts[0];
    int status = FWK_TRACE_START(0);
    assert(status == FWK_SUCCESS);

    current_trace_count = trace_counts[1];
    status = FWK_TRACE_START(1);
    assert(status == FWK_SUCCESS);
    current_trace_count = trace_counts[2];
    expected_id = 1;
    expected_trace_entry_count = trace_counts[2] - trace_counts[1];
    status = FWK_TRACE_FINISH(1, "");
    assert(status == FWK_SUCCESS);

    current_trace_count = trace_counts[3];
    expected_id = 0;
    expected_trace_entry_count = trace_counts[3] - trace_counts[0];
    status = FWK_TRACE_FINISH(0, "");
    assert(status == FWK_SUCCESS);
}

static void test_fwk_trace_partial_nesting(void)
{
    /*
     *  ----(1000)------(2000)----(3000)-----(4000)---------->
     *  ----[s:0]-------[s:1]-----[f:0]------[f:1]----------->
     */
    fwk_trace_count_t trace_counts[] = { 1000, 2000, 3000, 4000 };
    current_trace_count = trace_counts[0];
    int status = FWK_TRACE_START(0);
    assert(status == FWK_SUCCESS);

    current_trace_count = trace_counts[1];
    status = FWK_TRACE_START(1);
    assert(status == FWK_SUCCESS);

    current_trace_count = trace_counts[2];
    expected_id = 0;
    expected_trace_entry_count = trace_counts[2] - trace_counts[0];
    status = FWK_TRACE_FINISH(0, "");
    assert(status == FWK_SUCCESS);

    current_trace_count = trace_counts[3];
    expected_id = 1;
    expected_trace_entry_count = trace_counts[3] - trace_counts[1];
    status = FWK_TRACE_FINISH(1, "");
    assert(status == FWK_SUCCESS);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_trace_start_invalid_id),
    FWK_TEST_CASE(test_fwk_trace_start_twice),
    FWK_TEST_CASE(test_fwk_trace_start_no_driver),
    FWK_TEST_CASE(test_fwk_trace_finish_invalid_id),
    FWK_TEST_CASE(test_fwk_trace_finish_with_no_start),
    FWK_TEST_CASE(test_fwk_trace_normal_usage),
    FWK_TEST_CASE(test_fwk_trace_full_nesting),
    FWK_TEST_CASE(test_fwk_trace_partial_nesting),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_test",

    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,

    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
