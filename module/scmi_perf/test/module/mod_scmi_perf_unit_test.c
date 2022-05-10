/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <internal/scmi_perf.h>

#ifdef TEST_ON_TARGET
#    include <fwk_id.h>
#    include <fwk_module.h>
#else
#    include <Mockfwk_id.h>
#    include <Mockfwk_mm.h>
#    include <Mockfwk_module.h>

#    include <internal/Mockfwk_core_internal.h>
#endif
#include <Mockmod_scmi_perf_extra.h>
#include <config_scmi_perf.h>

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#ifdef BUILD_HAS_SCMI_PERF_PROTOCOL_OPS
#    include <scmi_perf_protocol_ops.c>
#endif

#include UNIT_TEST_SRC

void setUp(void)
{
    scmi_perf_ctx.config = config_scmi_perf.data;
    scmi_perf_ctx.domain_count = scmi_perf_ctx.config->perf_doms_count;
}

void tearDown(void)
{
}

void utest_scmi_perf_init_null_config(void)
{
    int status;

    status = scmi_perf_init(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF), 0, NULL);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_scmi_perf_init_empty_domains(void)
{
    int status;
    fwk_id_t module_id;
    struct mod_scmi_perf_config config;

    config.domains = NULL;

    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF);
    status = scmi_perf_init(module_id, 0, &config);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_scmi_perf_init_no_dvfs_domains(void)
{
    int status;
    fwk_id_t module_id;

    fwk_module_get_element_count_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_DVFS), 0);

    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF);
    status = scmi_perf_init(module_id, 0, &perf_config);

    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_scmi_perf_bind_round_one(void)
{
    int status;
    fwk_id_t id;

    id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF);
    status = scmi_perf_bind(id, 1);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_scmi_perf_bind_scmi_bind(void)
{
    int status;
    fwk_id_t id;

    id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF);

    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_E_ACCESS);
    status = scmi_perf_bind(id, 0);

    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

void utest_scmi_perf_process_bind_request_update_api_invalid_source(void)
{
    int status;
    fwk_id_t source_id, target_id, api_id;
    struct mod_scmi_to_protocol_api *api;

    source_id = fwk_module_id_scmi; /* Only DVFS can use this API */
    target_id = fwk_module_id_scmi_perf;
    api_id =
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, MOD_SCMI_PERF_DVFS_UPDATE_API);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_SCMI_PERF_DVFS_UPDATE_API);
    fwk_id_is_equal_ExpectAndReturn(source_id, fwk_module_id_dvfs, false);

    status = scmi_perf_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

void utest_scmi_perf_process_bind_request_update_api(void)
{
    int status;
    fwk_id_t source_id, target_id, api_id;
    struct mod_scmi_to_protocol_api *api;

    source_id = fwk_module_id_dvfs; /* Only DVFS can use this API */
    target_id = fwk_module_id_scmi_perf;
    api_id =
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, MOD_SCMI_PERF_DVFS_UPDATE_API);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_SCMI_PERF_DVFS_UPDATE_API);
    fwk_id_is_equal_ExpectAndReturn(source_id, fwk_module_id_dvfs, true);

    status = scmi_perf_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&perf_update_api, api);
}

void utest_scmi_perf_process_bind_request_invalid_api(void)
{
    int status;
    fwk_id_t source_id, target_id, api_id;
    struct mod_scmi_to_protocol_api *api;

    source_id = fwk_module_id_scmi;
    target_id = fwk_module_id_scmi_perf;
    api_id = FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, MOD_SCMI_PERF_API_COUNT);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_SCMI_PERF_API_COUNT);

    status = scmi_perf_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

int scmi_perf_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_scmi_perf_init_null_config);
    RUN_TEST(utest_scmi_perf_init_empty_domains);
    RUN_TEST(utest_scmi_perf_init_no_dvfs_domains);

    RUN_TEST(utest_scmi_perf_bind_round_one);
    RUN_TEST(utest_scmi_perf_bind_scmi_bind);

    RUN_TEST(utest_scmi_perf_process_bind_request_update_api_invalid_source);
    RUN_TEST(utest_scmi_perf_process_bind_request_update_api);

    RUN_TEST(utest_scmi_perf_process_bind_request_invalid_api);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_perf_test_main();
}
#endif
