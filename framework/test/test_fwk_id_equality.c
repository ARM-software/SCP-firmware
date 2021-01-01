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

static void test_equal_module_id(void)
{
    fwk_id_t left = FWK_ID_MODULE(42);
    fwk_id_t right = FWK_ID_MODULE(42);

    assert(fwk_id_is_equal(left, right));
}

static void test_not_equal_module_id(void)
{
    fwk_id_t left = FWK_ID_MODULE(42);
    fwk_id_t right = FWK_ID_MODULE(58);

    assert(!fwk_id_is_equal(left, right));
}

static void test_equal_element_id(void)
{
    fwk_id_t left = FWK_ID_ELEMENT(42, 58);
    fwk_id_t right = FWK_ID_ELEMENT(42, 58);

    assert(fwk_id_is_equal(left, right));
}

static void test_not_equal_element_id(void)
{
    fwk_id_t left = FWK_ID_ELEMENT(42, 58);
    fwk_id_t right = FWK_ID_ELEMENT(58, 42);

    assert(!fwk_id_is_equal(left, right));
}

static void test_equal_api_id(void)
{
    fwk_id_t left = FWK_ID_API(42, 14);
    fwk_id_t right = FWK_ID_API(42, 14);

    assert(fwk_id_is_equal(left, right));
}

static void test_not_equal_api_id(void)
{
    fwk_id_t left = FWK_ID_API(4, 8);
    fwk_id_t right = FWK_ID_API(8, 4);

    assert(!fwk_id_is_equal(left, right));
}

static void test_equal_event_id(void)
{
    fwk_id_t left = FWK_ID_EVENT(42, 45);
    fwk_id_t right = FWK_ID_EVENT(42, 45);

    assert(fwk_id_is_equal(left, right));
}

static void test_not_equal_event_id(void)
{
    fwk_id_t left = FWK_ID_EVENT(42, 58);
    fwk_id_t right = FWK_ID_EVENT(58, 42);

    assert(!fwk_id_is_equal(left, right));
}

static void test_equal_notification_id(void)
{
    fwk_id_t left = FWK_ID_NOTIFICATION(42, 45);
    fwk_id_t right = FWK_ID_NOTIFICATION(42, 45);

    assert(fwk_id_is_equal(left, right));
}

static void test_not_equal_notification_id(void)
{
    fwk_id_t left = FWK_ID_NOTIFICATION(42, 58);
    fwk_id_t right = FWK_ID_NOTIFICATION(58, 42);

    assert(!fwk_id_is_equal(left, right));
}

static void test_not_equal_shared_module(void)
{
    fwk_id_t left = FWK_ID_ELEMENT(42, 0);
    fwk_id_t right = FWK_ID_API(42, 0);

    assert(!fwk_id_is_equal(left, right));
}

static void test_defined_optional_id(void)
{
    fwk_optional_id_t opt_id = FWK_ID_NONE_INIT;

    assert(fwk_optional_id_is_defined(opt_id));
}

static void test_undefined_optional_id(void)
{
    fwk_optional_id_t opt_id = (fwk_optional_id_t){ 0 };

    assert(!fwk_optional_id_is_defined(opt_id));
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_equal_module_id),
    FWK_TEST_CASE(test_not_equal_module_id),
    FWK_TEST_CASE(test_equal_element_id),
    FWK_TEST_CASE(test_not_equal_element_id),
    FWK_TEST_CASE(test_equal_api_id),
    FWK_TEST_CASE(test_not_equal_api_id),
    FWK_TEST_CASE(test_equal_event_id),
    FWK_TEST_CASE(test_not_equal_event_id),
    FWK_TEST_CASE(test_equal_notification_id),
    FWK_TEST_CASE(test_not_equal_notification_id),
    FWK_TEST_CASE(test_not_equal_shared_module),
    FWK_TEST_CASE(test_defined_optional_id),
    FWK_TEST_CASE(test_undefined_optional_id),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_id_equality",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
