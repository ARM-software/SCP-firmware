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

#include <stdio.h>
#include <string.h>

static void test_format_module_id(void)
{
    fwk_id_t id = FWK_ID_MODULE(42);
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%s", FWK_ID_STR(id));

    assert(strcmp(buffer, "[MOD 42]") == 0);
}

static void test_format_element_id(void)
{
    fwk_id_t id = FWK_ID_ELEMENT(42, 58);
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%s", FWK_ID_STR(id));

    assert(strcmp(buffer, "[ELM 42:58]") == 0);
}

static void test_format_api_id(void)
{
    fwk_id_t id = FWK_ID_API(42, 14);
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%s", FWK_ID_STR(id));

    assert(strcmp(buffer, "[API 42:14]") == 0);
}

static void test_format_event_id(void)
{
    fwk_id_t id = FWK_ID_EVENT(42, 56);
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%s", FWK_ID_STR(id));

    assert(strcmp(buffer, "[EVT 42:56]") == 0);
}

static void test_format_notification_id(void)
{
    fwk_id_t id = FWK_ID_NOTIFICATION(42, 56);
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%s", FWK_ID_STR(id));

    assert(strcmp(buffer, "[NOT 42:56]") == 0);
}

static void test_format_max_length(void)
{
    fwk_id_t id = FWK_ID_ELEMENT(0xFF, 0xFFF);
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%s", FWK_ID_STR(id));

    assert(strcmp(buffer, "[ELM 255:4095]") == 0);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_format_module_id),
    FWK_TEST_CASE(test_format_element_id),
    FWK_TEST_CASE(test_format_api_id),
    FWK_TEST_CASE(test_format_event_id),
    FWK_TEST_CASE(test_format_notification_id),
    FWK_TEST_CASE(test_format_max_length),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_id_format",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
