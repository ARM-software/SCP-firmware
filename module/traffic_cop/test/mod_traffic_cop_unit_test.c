/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockfwk_notification.h>
#include <config_traffic_cop.h>

#include <internal/Mockfwk_core_internal.h>

#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>

#include UNIT_TEST_SRC

static struct mod_tcop_domain_ctx dev_ctx_table[1];
struct mod_tcop_core_ctx core_ctx_table[2];

void setUp(void)
{
    struct mod_tcop_core_config const *core_config;
    struct mod_tcop_domain_ctx *domain_ctx;
    struct mod_tcop_core_ctx *core_ctx;
    uint32_t core_idx;

    tcop_ctx.tcop_domain_count = 1;
    tcop_ctx.domain_ctx = domain_ctx = &dev_ctx_table[0];

    domain_ctx->domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_TRAFFIC_COP, 0);
    domain_ctx->num_cores = 2;
    domain_ctx->domain_config = &fake_domain_conf[0];
    domain_ctx->core_ctx = &core_ctx_table[0];
    domain_ctx->num_cores_online = 0;

    /* Initialize each core */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        core_config = &domain_ctx->domain_config->core_config[core_idx];

        if (core_config->core_starts_online) {
            domain_ctx->num_cores_online++;
            core_ctx->online = true;
        }
    }
}

void tearDown(void)
{
}

void test_function_tcop_start_mod_id_success(void)
{
    int status;

    fwk_module_is_valid_module_id_ExpectAndReturn(
        fwk_module_id_traffic_cop, true);

    status = tcop_start(fwk_module_id_traffic_cop);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_function_tcop_start_notif_subscribe_success(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_TRAFFIC_COP, 0);

    fwk_module_is_valid_module_id_ExpectAnyArgsAndReturn(false);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_notification_subscribe_IgnoreAndReturn(FWK_SUCCESS);

    status = tcop_start(elem_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    fwk_notification_subscribe_StopIgnore();
}

int fwk_notif_sub_cback_fail_subscribe_pre_transition(
    fwk_id_t notification_id,
    fwk_id_t source_id,
    fwk_id_t target_id,
    int cmock_num_calls)
{
    if (cmock_num_calls == 0) {
        return FWK_E_STATE;
    }

    return FWK_SUCCESS;
}

int fwk_notif_sub_cback_fail_subscribe_post_transition(
    fwk_id_t notification_id,
    fwk_id_t source_id,
    fwk_id_t target_id,
    int cmock_num_calls)
{
    if (cmock_num_calls == 2) {
        return FWK_E_STATE;
    }

    return FWK_SUCCESS;
}

void test_function_tcop_start_notif_subscribe_fail_1st_call(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_TRAFFIC_COP, 0);

    fwk_module_is_valid_module_id_IgnoreAndReturn(false);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_notification_subscribe_Stub(
        fwk_notif_sub_cback_fail_subscribe_pre_transition);

    status = tcop_start(elem_id);
    TEST_ASSERT_EQUAL(status, FWK_E_STATE);
}

void test_function_tcop_start_notif_subscribe_fail_2nd_call(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_TRAFFIC_COP, 0);

    fwk_module_is_valid_module_id_IgnoreAndReturn(false);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_notification_subscribe_Stub(
        fwk_notif_sub_cback_fail_subscribe_post_transition);

    status = tcop_start(elem_id);
    TEST_ASSERT_EQUAL(status, FWK_E_STATE);
}

void test_function_tcop_start_elem_id_domain_limits_init_success(void)
{
    int status;
    struct mod_tcop_domain_ctx *domain_ctx;
    struct mod_tcop_pct_table *pct;

    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_TRAFFIC_COP, 0);

    fwk_module_is_valid_module_id_IgnoreAndReturn(false);
    fwk_id_get_element_idx_IgnoreAndReturn(0);
    fwk_notification_subscribe_IgnoreAndReturn(FWK_SUCCESS);

    status = tcop_start(elem_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    domain_ctx = &dev_ctx_table[0];
    pct = &fake_pct_table[0];

    TEST_ASSERT_EQUAL(pct[1].perf_limit, domain_ctx->perf_limit);
    fwk_module_is_valid_module_id_StopIgnore();
    fwk_id_get_element_idx_StopIgnore();
    fwk_notification_subscribe_StopIgnore();
}

int mod_tcop_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_function_tcop_start_mod_id_success);

    RUN_TEST(test_function_tcop_start_notif_subscribe_success);

    RUN_TEST(test_function_tcop_start_notif_subscribe_fail_1st_call);
    RUN_TEST(test_function_tcop_start_notif_subscribe_fail_2nd_call);

    RUN_TEST(test_function_tcop_start_elem_id_domain_limits_init_success);

    return UNITY_END();
}

int main(void)
{
    return mod_tcop_test_main();
}
