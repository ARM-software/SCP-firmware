/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_test.h>

static void test_is_module_id(void)
{
    fwk_id_t id = FWK_ID_MODULE(42);

    assert(fwk_id_is_type(id, FWK_ID_TYPE_MODULE));
}

static void test_is_element_id(void)
{
    fwk_id_t id = FWK_ID_ELEMENT(42, 58);

    assert(fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT));
}

static void test_is_api_id(void)
{
    fwk_id_t id = FWK_ID_API(42, 14);

    assert(fwk_id_is_type(id, FWK_ID_TYPE_API));
}

static void test_is_event_id(void)
{
    fwk_id_t id = FWK_ID_EVENT(42, 14);

    assert(fwk_id_is_type(id, FWK_ID_TYPE_EVENT));
}

static void test_is_notification_id(void)
{
    fwk_id_t id = FWK_ID_NOTIFICATION(42, 14);

    assert(fwk_id_is_type(id, FWK_ID_TYPE_NOTIFICATION));
}

static void test_type_of_module_id(void)
{
    fwk_id_t id = FWK_ID_MODULE(42);

    assert(fwk_id_get_type(id) == FWK_ID_TYPE_MODULE);
}

static void test_type_of_element_id(void)
{
    fwk_id_t id = FWK_ID_ELEMENT(42, 58);

    assert(fwk_id_get_type(id) == FWK_ID_TYPE_ELEMENT);
}

static void test_type_of_api_id(void)
{
    fwk_id_t id = FWK_ID_API(42, 14);

    assert(fwk_id_get_type(id) == FWK_ID_TYPE_API);
}

static void test_type_of_event_id(void)
{
    fwk_id_t id = FWK_ID_EVENT(42, 56);

    assert(fwk_id_get_type(id) == FWK_ID_TYPE_EVENT);
}

static void test_type_of_notification_id(void)
{
    fwk_id_t id = FWK_ID_NOTIFICATION(42, 56);

    assert(fwk_id_get_type(id) == FWK_ID_TYPE_NOTIFICATION);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_is_module_id),
    FWK_TEST_CASE(test_is_element_id),
    FWK_TEST_CASE(test_is_api_id),
    FWK_TEST_CASE(test_is_event_id),
    FWK_TEST_CASE(test_is_notification_id),
    FWK_TEST_CASE(test_type_of_module_id),
    FWK_TEST_CASE(test_type_of_element_id),
    FWK_TEST_CASE(test_type_of_api_id),
    FWK_TEST_CASE(test_type_of_event_id),
    FWK_TEST_CASE(test_type_of_notification_id),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_id_type",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
