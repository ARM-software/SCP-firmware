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

static void test_overlapping_fields(void)
{
    fwk_id_t id = FWK_ID_MODULE_INIT(42);

    assert(id.common.type == id.element.type);
    assert(id.common.type == id.api.type);
    assert(id.common.type == id.event.type);
    assert(id.common.type == id.notification.type);
    assert(id.common.module_idx == id.element.module_idx);
    assert(id.common.module_idx == id.api.module_idx);
    assert(id.common.module_idx == id.event.module_idx);
    assert(id.common.module_idx == id.notification.module_idx);
}

static void test_build_module(void)
{
    fwk_id_t id = FWK_ID_MODULE_INIT(42);

    assert(id.common.type == __FWK_ID_TYPE_MODULE);
    assert(id.common.module_idx == 42);

    assert(fwk_id_is_equal(id, FWK_ID_MODULE(42)));
}

static void test_build_element(void)
{
    fwk_id_t id = FWK_ID_ELEMENT_INIT(42, 58);

    assert(id.element.type == __FWK_ID_TYPE_ELEMENT);
    assert(id.element.module_idx == 42);
    assert(id.element.element_idx == 58);

    assert(fwk_id_is_equal(id, FWK_ID_ELEMENT(42, 58)));
}

static void test_build_api(void)
{
    fwk_id_t id = FWK_ID_API_INIT(42, 14);

    assert(id.api.type == __FWK_ID_TYPE_API);
    assert(id.api.module_idx == 42);
    assert(id.api.api_idx == 14);

    assert(fwk_id_is_equal(id, FWK_ID_API(42, 14)));
}

static void test_build_event(void)
{
    fwk_id_t id = FWK_ID_EVENT_INIT(42, 56);

    assert(id.event.type == __FWK_ID_TYPE_EVENT);
    assert(id.event.module_idx == 42);
    assert(id.event.event_idx == 56);

    assert(fwk_id_is_equal(id, FWK_ID_EVENT(42, 56)));
}

static void test_build_notification(void)
{
    fwk_id_t id = FWK_ID_NOTIFICATION_INIT(42, 56);

    assert(id.event.type == __FWK_ID_TYPE_NOTIFICATION);
    assert(id.notification.module_idx == 42);
    assert(id.notification.notification_idx == 56);

    assert(fwk_id_is_equal(id, FWK_ID_NOTIFICATION(42, 56)));
}

static void test_module_from_module(void)
{
    fwk_id_t id = FWK_ID_MODULE_INIT(42);
    fwk_id_t parent_id = fwk_id_build_module_id(id);

    assert(parent_id.common.type == __FWK_ID_TYPE_MODULE);
    assert(parent_id.common.module_idx == 42);

    assert(fwk_id_is_equal(id, FWK_ID_MODULE(42)));
}

static void test_module_from_element(void)
{
    fwk_id_t id = FWK_ID_ELEMENT_INIT(42, 58);
    fwk_id_t parent_id = fwk_id_build_module_id(id);

    assert(parent_id.common.type == __FWK_ID_TYPE_MODULE);
    assert(parent_id.common.module_idx == 42);

    assert(fwk_id_is_equal(id, FWK_ID_ELEMENT(42, 58)));
}

static void test_element_from_module_and_index(void)
{
    fwk_id_t id = FWK_ID_MODULE_INIT(42);
    fwk_id_t element_id = fwk_id_build_element_id(id, 58);

    assert(fwk_id_is_equal(element_id, FWK_ID_ELEMENT(42, 58)));
}

static void test_element_from_element_and_index(void)
{
    fwk_id_t id = FWK_ID_ELEMENT_INIT(42, 64);
    fwk_id_t element_id = fwk_id_build_element_id(id, 58);

    assert(fwk_id_is_equal(element_id, FWK_ID_ELEMENT(42, 58)));
}

static void test_sub_element_from_element_and_index(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(42, 64);
    fwk_id_t sub_element_id = fwk_id_build_sub_element_id(element_id, 58);

    assert(fwk_id_is_equal(sub_element_id, FWK_ID_SUB_ELEMENT(42, 64, 58)));
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_overlapping_fields),
    FWK_TEST_CASE(test_build_module),
    FWK_TEST_CASE(test_build_element),
    FWK_TEST_CASE(test_build_api),
    FWK_TEST_CASE(test_build_event),
    FWK_TEST_CASE(test_build_notification),
    FWK_TEST_CASE(test_module_from_module),
    FWK_TEST_CASE(test_module_from_element),
    FWK_TEST_CASE(test_element_from_module_and_index),
    FWK_TEST_CASE(test_element_from_element_and_index),
    FWK_TEST_CASE(test_sub_element_from_element_and_index),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_id_build",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
