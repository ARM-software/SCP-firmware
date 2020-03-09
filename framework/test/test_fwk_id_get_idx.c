/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_test.h>

#include <stddef.h>

static void test_module_idx_of_module_id(void)
{
    fwk_id_t id = FWK_ID_MODULE(42);

    assert(fwk_id_get_module_idx(id) == 42);
}

static void test_element_idx_of_element_id(void)
{
    fwk_id_t id = FWK_ID_ELEMENT(42, 45);

    assert(fwk_id_get_element_idx(id) == 45);
}

static void test_api_idx_of_api_id(void)
{
    fwk_id_t id = FWK_ID_API(42, 10);

    assert(fwk_id_get_api_idx(id) == 10);
}

static void test_event_idx_of_event_id(void)
{
    fwk_id_t id = FWK_ID_EVENT(42, 58);

    assert(fwk_id_get_event_idx(id) == 58);
}

static void test_notification_idx_of_notification_id(void)
{
    fwk_id_t id = FWK_ID_NOTIFICATION(42, 58);

    assert(fwk_id_get_notification_idx(id) == 58);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_module_idx_of_module_id),
    FWK_TEST_CASE(test_element_idx_of_element_id),
    FWK_TEST_CASE(test_api_idx_of_api_id),
    FWK_TEST_CASE(test_event_idx_of_event_id),
    FWK_TEST_CASE(test_notification_idx_of_notification_id),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_id_get_idx",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
