/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_core.h>
#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockfwk_notification.h>
#include <config_mpmm.h>

#include <internal/Mockfwk_core_internal.h>

#include <mod_power_domain.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>

#include UNIT_TEST_SRC

uint64_t fake_amu_counter[AMU_COUNT] = {
    0xAAAA1111AAAA1111, 0xAAAA2222AAAA2222, 0xAAAA3333AAAA3333,
    0xAAAA4444AAAA4444, 0xAAAA5555AAAA5555, 0xAAAA6666AAAA6666,
    0xAAAA7777AAAA7777,
};

int fake_plugin_set_limits(struct plugin_limits_req *data)
{
    return FWK_SUCCESS;
}

int amu_mmap_copy_data(
    fwk_id_t start_counter_id,
    uint64_t *counter_buff,
    size_t num_counter)
{
    memcpy(
        counter_buff,
        &fake_amu_counter[start_counter_id.sub_element.sub_element_idx],
        sizeof(uint64_t) * num_counter);
    return FWK_SUCCESS;
}

int amu_mmap_return_error(
    fwk_id_t start_counter_id,
    uint64_t *counter_buff,
    size_t num_counter)
{
    return FWK_E_RANGE;
}

uint32_t adj_max_limit = 0xFF;
struct mod_mpmm_domain_ctx dev_ctx_table[1];
struct perf_plugins_perf_update perf_update = {
    .domain_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, 0, 0),
    .adj_max_limit = &adj_max_limit,
};
struct perf_plugins_perf_report fake_perf_report = {
    .dep_dom_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0),
    .level = 0xFF,
};
struct perf_plugins_handler_api handler_api = {
    .plugin_set_limits = fake_plugin_set_limits,
};
struct amu_api amu_api = {
    .get_counters = amu_mmap_copy_data,
};

void setUp(void)
{
    struct mod_mpmm_domain_ctx *domain_ctx;

    mpmm_ctx.mpmm_domain_count = 1;
    mpmm_ctx.domain_ctx = domain_ctx = &dev_ctx_table[0];
    mpmm_ctx.perf_plugins_handler_api = &handler_api;
    mpmm_ctx.amu_driver_api = &amu_api;
    mpmm_ctx.amu_driver_api_id =
        FWK_ID_API(FWK_MODULE_IDX_AMU_MMAP, MOD_AMU_MMAP_API_IDX_AMU);

    domain_ctx->domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);
    domain_ctx->num_cores = 1;
    domain_ctx->num_cores_online = 0;
    domain_ctx->perf_limit = 1;
    domain_ctx->wait_for_perf_transition = true;
    domain_ctx->domain_config = &fake_dom_conf[MPMM_DOM_DEFAULT];
}

void tearDown(void)
{
}

void utest_mpmm_start_mod_id_success(void)
{
    int status;

    fwk_module_is_valid_module_id_ExpectAndReturn(fwk_module_id_mpmm, true);

    status = mpmm_start(fwk_module_id_mpmm);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mpmm_start_notif_subscribe_success(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_module_is_valid_module_id_ExpectAndReturn(elem_id, false);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_notification_subscribe_ExpectAndReturn(
        mod_pd_notification_id_power_state_pre_transition,
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        mpmm_ctx.domain_ctx->domain_id,
        FWK_SUCCESS);
    fwk_notification_subscribe_ExpectAndReturn(
        mod_pd_notification_id_power_state_transition,
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        mpmm_ctx.domain_ctx->domain_id,
        FWK_SUCCESS);

    status = mpmm_start(elem_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mpmm_start_notif_subscribe_pre_transition_fail(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_module_is_valid_module_id_ExpectAndReturn(elem_id, false);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);

    fwk_notification_subscribe_ExpectAndReturn(
        mod_pd_notification_id_power_state_pre_transition,
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        mpmm_ctx.domain_ctx->domain_id,
        FWK_E_STATE);

    status = mpmm_start(elem_id);
    TEST_ASSERT_EQUAL(FWK_E_STATE, status);
}

void utest_mpmm_start_notif_subscribe_post_transition_fail(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_module_is_valid_module_id_ExpectAndReturn(elem_id, false);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_notification_subscribe_ExpectAndReturn(
        mod_pd_notification_id_power_state_pre_transition,
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        mpmm_ctx.domain_ctx->domain_id,
        FWK_SUCCESS);
    fwk_notification_subscribe_ExpectAndReturn(
        mod_pd_notification_id_power_state_transition,
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        mpmm_ctx.domain_ctx->domain_id,
        FWK_E_STATE);

    status = mpmm_start(elem_id);
    TEST_ASSERT_EQUAL(FWK_E_STATE, status);
}

void utest_mpmm_init_success(void)
{
    int status;
    unsigned int count = 1;
    struct mod_mpmm_domain_ctx domain_ctx;
    fwk_id_t api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_AMU_MMAP, MOD_AMU_MMAP_API_IDX_AMU);

    fwk_mm_calloc_ExpectAndReturn(
        count, sizeof(struct mod_mpmm_domain_ctx), &domain_ctx);

    status = mpmm_init(fwk_module_id_mpmm, count, &api_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(count, mpmm_ctx.mpmm_domain_count);
    TEST_ASSERT_EQUAL_PTR(&domain_ctx, mpmm_ctx.domain_ctx);
    TEST_ASSERT_EQUAL(api_id.value, mpmm_ctx.amu_driver_api_id.value);
}

void utest_mpmm_init_element_count_fail(void)
{
    int status;

    status = mpmm_init(fwk_module_id_mpmm, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_mpmm_element_init_two_core_success(void)
{
    int status;
    uint64_t cached_counters[CORE_IDX_COUNT];
    uint64_t delta[CORE_IDX_COUNT];
    unsigned int count = CORE_IDX_COUNT;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_id_build_sub_element_id_IgnoreAndReturn(elem_id);
    fwk_mm_calloc_ExpectAndReturn(
        dev_ctx_table[0].domain_config->num_threshold_counters,
        sizeof(uint32_t),
        &cached_counters[CORE0_IDX]);
    fwk_mm_calloc_ExpectAndReturn(
        dev_ctx_table[0].domain_config->num_threshold_counters,
        sizeof(uint32_t),
        &delta[CORE0_IDX]);
    fwk_mm_calloc_ExpectAndReturn(
        dev_ctx_table[0].domain_config->num_threshold_counters,
        sizeof(uint32_t),
        &cached_counters[CORE1_IDX]);
    fwk_mm_calloc_ExpectAndReturn(
        dev_ctx_table[0].domain_config->num_threshold_counters,
        sizeof(uint32_t),
        &delta[CORE1_IDX]);

    status =
        mpmm_element_init(elem_id, count, &fake_dom_conf[MPMM_DOM_DEFAULT]);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(count, dev_ctx_table[0].num_cores);
    TEST_ASSERT_EQUAL(false, dev_ctx_table[0].wait_for_perf_transition);
    TEST_ASSERT_EQUAL_PTR(
        &fake_dom_conf[MPMM_DOM_DEFAULT], dev_ctx_table[0].domain_config);

    /* CORE0_IDX */
    TEST_ASSERT_EQUAL_PTR(
        fake_core_config[0].mpmm_reg_base,
        dev_ctx_table[0].core_ctx[CORE0_IDX].mpmm);
    TEST_ASSERT_EQUAL(
        fake_core_config[CORE0_IDX].base_aux_counter_id.value,
        dev_ctx_table[0].core_ctx[CORE0_IDX].base_aux_counter_id.value);
    TEST_ASSERT_EQUAL_PTR(
        &cached_counters[CORE0_IDX],
        dev_ctx_table[0].core_ctx[CORE0_IDX].cached_counters);
    TEST_ASSERT_EQUAL_PTR(
        &delta[CORE0_IDX], dev_ctx_table[0].core_ctx[CORE0_IDX].delta);

    /* CORE1_IDX */
    TEST_ASSERT_EQUAL_PTR(
        fake_core_config[CORE1_IDX].mpmm_reg_base,
        dev_ctx_table[0].core_ctx[CORE1_IDX].mpmm);
    TEST_ASSERT_EQUAL(
        fake_core_config[CORE1_IDX].base_aux_counter_id.value,
        dev_ctx_table[0].core_ctx[CORE1_IDX].base_aux_counter_id.value);
    TEST_ASSERT_EQUAL_PTR(
        &cached_counters[CORE1_IDX],
        dev_ctx_table[0].core_ctx[CORE1_IDX].cached_counters);
    TEST_ASSERT_EQUAL_PTR(
        &delta[CORE1_IDX], dev_ctx_table[0].core_ctx[CORE1_IDX].delta);
}

void utest_mpmm_element_init_element_count_0_fail(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    status = mpmm_element_init(elem_id, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_mpmm_element_init_element_count_max_fail(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    status =
        mpmm_element_init(elem_id, (MPMM_MAX_NUM_CORES_IN_DOMAIN + 1), NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_mpmm_element_init_max_threshold_count_fail(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);

    status = mpmm_element_init(
        elem_id, 1, &fake_dom_conf[MPMM_DOM_MAX_THRESHOLD_COUNT]);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_mpmm_element_init_num_threshold_mismatch(void)
{
    int status;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, 0);
    fwk_id_build_sub_element_id_IgnoreAndReturn(elem_id);

    status = mpmm_element_init(
        elem_id, 1, &fake_dom_conf[MPMM_DOM_NUM_THRESHOLD_NOT_EQUAL]);
    TEST_ASSERT_EQUAL(FWK_E_DEVICE, status);
}

void utest_mpmm_bind_first_round_success(void)
{
    int status;
    unsigned int round = 0;

    status = mpmm_bind(fwk_module_id_mpmm, round);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mpmm_bind_invalid_module_success(void)
{
    int status;
    unsigned int round = 1;

    fwk_module_is_valid_module_id_ExpectAndReturn(fwk_module_id_mpmm, false);

    status = mpmm_bind(fwk_module_id_mpmm, round);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mpmm_bind_module_bind_success(void)
{
    int status;
    unsigned int round = 1;

    fwk_module_is_valid_module_id_ExpectAndReturn(fwk_module_id_mpmm, true);
    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(mpmm_ctx.amu_driver_api_id.common.module_idx),
        mpmm_ctx.amu_driver_api_id,
        &mpmm_ctx.amu_driver_api,
        FWK_SUCCESS);
    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF),
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, MOD_SCMI_PERF_PLUGINS_API),
        &mpmm_ctx.perf_plugins_handler_api,
        FWK_SUCCESS);

    status = mpmm_bind(fwk_module_id_mpmm, round);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mpmm_bind_amu_bind_fail(void)
{
    int status;
    unsigned int round = 1;

    fwk_module_is_valid_module_id_ExpectAndReturn(fwk_module_id_mpmm, true);
    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(mpmm_ctx.amu_driver_api_id.common.module_idx),
        mpmm_ctx.amu_driver_api_id,
        &mpmm_ctx.amu_driver_api,
        FWK_E_PARAM);

    status = mpmm_bind(fwk_module_id_mpmm, round);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_mpmm_process_bind_request_success(void)
{
    int status;
    const void *api = NULL;

    fwk_id_is_equal_ExpectAndReturn(
        fwk_module_id_scmi_perf, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF), true);

    status = mpmm_process_bind_request(
        fwk_module_id_scmi_perf, fwk_module_id_mpmm, fwk_module_id_mpmm, &api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(&perf_plugins_api, api);
}

void utest_mpmm_process_bind_request_id_not_equal(void)
{
    int status;

    fwk_id_is_equal_ExpectAndReturn(
        fwk_module_id_mpmm, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF), false);

    status = mpmm_process_bind_request(
        fwk_module_id_mpmm, fwk_module_id_mpmm, fwk_module_id_mpmm, NULL);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

void utest_mpmm_process_notification_pre_state_to_on(void)
{
    int status;
    struct mod_pd_power_state_pre_transition_notification_params *params;
    struct fwk_event event = { .target_id =
                                   FWK_ID_EVENT(FWK_MODULE_IDX_MPMM, 0) };
    struct fwk_event resp_event = { 0 };

    dev_ctx_table[0].num_cores_online = 0;
    dev_ctx_table[0].core_ctx[0].online = true;
    dev_ctx_table[0].threshold_map = 1;
    dev_ctx_table[0].current_perf_level =
        fake_pct_table[FAKE_PCT_TABLE_COUNT - 1].default_perf_limit + 1;
    dev_ctx_table[0].core_ctx[0].pd_blocked = false;
    params = (struct mod_pd_power_state_pre_transition_notification_params *)
                 event.params;
    params->current_state = MOD_PD_STATE_OFF;
    params->target_state = MOD_PD_STATE_ON;

    fwk_module_is_valid_element_id_ExpectAndReturn(event.target_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        event.source_id,
        true);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_pd_notification_id_power_state_pre_transition, true);

    status = mpmm_process_notification(&event, &resp_event);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(1, mpmm_ctx.domain_ctx->num_cores_online);
    TEST_ASSERT_EQUAL(true, mpmm_ctx.domain_ctx->core_ctx[0].online);
    TEST_ASSERT_EQUAL(
        (1 << MPMM_THRESHOLD_MAP_NUM_OF_BITS),
        mpmm_ctx.domain_ctx->threshold_map);
    TEST_ASSERT_EQUAL(
        fake_pct_table[FAKE_PCT_TABLE_COUNT - 1].default_perf_limit,
        mpmm_ctx.domain_ctx->perf_limit);
    TEST_ASSERT_EQUAL(true, mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked);
    TEST_ASSERT_EQUAL(true, mpmm_ctx.domain_ctx->wait_for_perf_transition);
    TEST_ASSERT_EQUAL(true, resp_event.is_delayed_response);
    TEST_ASSERT_EQUAL(event.cookie, mpmm_ctx.domain_ctx->core_ctx[0].cookie);
}

void utest_mpmm_process_notification_post_state_to_off(void)
{
    int status;
    struct mod_pd_power_state_transition_notification_params *params;
    struct fwk_event event = { .target_id =
                                   FWK_ID_EVENT(FWK_MODULE_IDX_MPMM, 0) };
    struct fwk_event resp_event = { 0 };

    mpmm_ctx.domain_ctx->num_cores_online = 1;
    params = (struct mod_pd_power_state_transition_notification_params *)
                 event.params;
    params->state = MOD_PD_STATE_OFF;

    fwk_module_is_valid_element_id_ExpectAndReturn(event.target_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        event.source_id,
        true);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_pd_notification_id_power_state_pre_transition, false);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_pd_notification_id_power_state_transition, true);

    status = mpmm_process_notification(&event, &resp_event);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(0, mpmm_ctx.domain_ctx->num_cores_online);
    TEST_ASSERT_EQUAL(false, mpmm_ctx.domain_ctx->core_ctx[0].online);
}

void utest_mpmm_process_notification_core_idx_larger(void)
{
    int status;
    struct fwk_event event = { .target_id =
                                   FWK_ID_EVENT(FWK_MODULE_IDX_MPMM, 0) };
    struct fwk_event resp_event = { 0 };

    fwk_module_is_valid_element_id_ExpectAndReturn(event.target_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        event.source_id,
        false);

    status = mpmm_process_notification(&event, &resp_event);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_mpmm_process_notification_no_perf_change(void)
{
    int status;
    struct mod_pd_power_state_pre_transition_notification_params *params;
    struct fwk_event event = { .target_id =
                                   FWK_ID_EVENT(FWK_MODULE_IDX_MPMM, 0) };
    struct fwk_event resp_event = { 0 };

    dev_ctx_table[0].num_cores_online = 0;
    dev_ctx_table[0].core_ctx[0].online = true;
    dev_ctx_table[0].threshold_map = 1;
    dev_ctx_table[0].current_perf_level = 0;
    dev_ctx_table[0].core_ctx[0].pd_blocked = false;

    params = (struct mod_pd_power_state_pre_transition_notification_params *)
                 event.params;
    params->target_state = MOD_PD_STATE_ON;

    fwk_module_is_valid_element_id_ExpectAndReturn(event.target_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->core_config[0].pd_id,
        event.source_id,
        true);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_pd_notification_id_power_state_pre_transition, true);

    status = mpmm_process_notification(&event, &resp_event);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(1, mpmm_ctx.domain_ctx->num_cores_online);
    TEST_ASSERT_EQUAL(true, mpmm_ctx.domain_ctx->core_ctx[0].online);
    TEST_ASSERT_EQUAL(
        (1 << MPMM_THRESHOLD_MAP_NUM_OF_BITS),
        mpmm_ctx.domain_ctx->threshold_map);
    TEST_ASSERT_EQUAL(
        fake_pct_table[FAKE_PCT_TABLE_COUNT - 1].default_perf_limit,
        mpmm_ctx.domain_ctx->perf_limit);
    TEST_ASSERT_EQUAL(false, mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked);
}

void utest_mpmm_update_success(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    mpmm_ctx.domain_ctx->num_cores_online = 1;
    *perf_update.adj_max_limit = 0xFF;

    fwk_id_get_element_idx_ExpectAndReturn(perf_update.domain_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);

    status = mpmm_update(&perf_update);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        mpmm_ctx.domain_ctx->perf_limit, *perf_update.adj_max_limit);
}

void utest_mpmm_update_domain_idx_not_found(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);
    *perf_update.adj_max_limit = 0xFF;

    fwk_id_get_element_idx_ExpectAndReturn(perf_update.domain_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, false);

    status = mpmm_update(&perf_update);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(0xFF, *perf_update.adj_max_limit);
}

void utest_mpmm_update_no_cores_online(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);
    dev_ctx_table[0].num_cores_online = 0;
    *perf_update.adj_max_limit = 0xFF;

    fwk_id_get_element_idx_ExpectAndReturn(perf_update.domain_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);

    status = mpmm_update(&perf_update);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(0xFF, *perf_update.adj_max_limit);
}

void utest_mpmm_update_wait_for_perf_transition(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    mpmm_ctx.domain_ctx->num_cores_online = 1;
    mpmm_ctx.domain_ctx->current_perf_level =
        mpmm_ctx.domain_ctx->perf_limit + 1;
    mpmm_ctx.domain_ctx->wait_for_perf_transition = false;
    *perf_update.adj_max_limit = 0xFF;

    fwk_id_get_element_idx_ExpectAndReturn(perf_update.domain_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);

    status = mpmm_update(&perf_update);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        mpmm_ctx.domain_ctx->perf_limit, *perf_update.adj_max_limit);
    TEST_ASSERT_EQUAL(true, mpmm_ctx.domain_ctx->wait_for_perf_transition);
}

void utest_mpmm_report_success(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    mpmm_ctx.domain_ctx->wait_for_perf_transition = true;
    dev_ctx_table[0].core_ctx[0].pd_blocked = false;
    dev_ctx_table[0].current_perf_level = 0;

    fwk_id_get_element_idx_ExpectAndReturn(fake_perf_report.dep_dom_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);

    status = mpmm_report(&fake_perf_report);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        fake_perf_report.level, mpmm_ctx.domain_ctx->current_perf_level);
    TEST_ASSERT_EQUAL(false, mpmm_ctx.domain_ctx->wait_for_perf_transition);
    TEST_ASSERT_EQUAL(false, mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked);
}

void utest_mpmm_report_domain_idx_not_found(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    fwk_id_get_element_idx_ExpectAndReturn(fake_perf_report.dep_dom_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, false);

    status = mpmm_report(&fake_perf_report);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_mpmm_report_no_wait_for_perf_transition(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    mpmm_ctx.domain_ctx->wait_for_perf_transition = false;
    dev_ctx_table[0].current_perf_level = 0;

    fwk_id_get_element_idx_ExpectAndReturn(fake_perf_report.dep_dom_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);

    status = mpmm_report(&fake_perf_report);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        fake_perf_report.level, mpmm_ctx.domain_ctx->current_perf_level);
}

void utest_mpmm_report_pd_blocked_delayed_resp_fail(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    fwk_id_get_element_idx_ExpectAndReturn(fake_perf_report.dep_dom_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);
    fwk_get_delayed_response_ExpectAnyArgsAndReturn(FWK_E_PARAM);

    mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked = true;

    status = mpmm_report(&fake_perf_report);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(false, mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked);
}

void utest_mpmm_report_pd_blocked_put_event_fail(void)
{
    int status;
    fwk_id_t perf_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0);

    fwk_id_get_element_idx_ExpectAndReturn(fake_perf_report.dep_dom_id, 0);
    fwk_id_is_equal_ExpectAndReturn(
        mpmm_ctx.domain_ctx->domain_config->perf_id, perf_id, true);
    fwk_get_delayed_response_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_E_PARAM);

    mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked = true;

    status = mpmm_report(&fake_perf_report);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(false, mpmm_ctx.domain_ctx->core_ctx[0].pd_blocked);
}

void utest_mpmm_core_evaluate_threshold_success(void)
{
    uint64_t cached_counters = 0;
    uint64_t delta = 0;
    struct mpmm_reg mpmm = { .MPMMCR = MPMM_MPMMCR_EN_MASK };
    struct mod_mpmm_core_ctx core_ctx_table;

    core_ctx_table.mpmm = &mpmm;
    core_ctx_table.base_aux_counter_id =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_AMU_MMAP, CORE0_IDX, AMU_AUX0);
    core_ctx_table.cached_counters = &cached_counters;
    core_ctx_table.delta = &delta;
    core_ctx_table.threshold = 0xFFFFFFFF;

    mpmm_core_evaluate_threshold(&dev_ctx_table[0], &core_ctx_table);
    TEST_ASSERT_EQUAL(
        (dev_ctx_table[0].domain_config->num_threshold_counters - 1),
        core_ctx_table.threshold);
    TEST_ASSERT_EQUAL(
        fake_amu_counter[AMU_AUX0], *core_ctx_table.cached_counters);
    TEST_ASSERT_EQUAL(fake_amu_counter[AMU_AUX0], *core_ctx_table.delta);
}

void utest_mpmm_core_evaluate_threshold_counter_not_enabled(void)
{
    struct mpmm_reg mpmm = { .MPMMCR = 0 };
    struct mod_mpmm_core_ctx core_ctx_table;

    core_ctx_table.mpmm = &mpmm;

    mpmm_core_evaluate_threshold(&dev_ctx_table[0], &core_ctx_table);
    TEST_ASSERT_EQUAL(
        dev_ctx_table[0].domain_config->num_threshold_counters,
        core_ctx_table.threshold);
}

void utest_mpmm_domain_set_thresholds_success(void)
{
    struct mpmm_reg mpmm = { 0 };

    dev_ctx_table[0].core_ctx[0].mpmm = &mpmm;
    dev_ctx_table[0].core_ctx[0].online = true;

    mpmm_domain_set_thresholds(&dev_ctx_table[0]);
    TEST_ASSERT_EQUAL(
        (1 << MPMM_MPMMCR_GEAR_POS), dev_ctx_table[0].core_ctx[0].mpmm->MPMMCR);
}

void utest_get_domain_ctx_null(void)
{
    struct mod_mpmm_domain_ctx *domain_ctx;
    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MPMM, 0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, mpmm_ctx.mpmm_domain_count);

    domain_ctx = get_domain_ctx(elem_id);
    TEST_ASSERT_EQUAL(NULL, domain_ctx);
}

void utest_mpmm_core_threshold_policy_highest_gear(void)
{
    uint32_t gear;
    uint64_t delta = 0;
    struct mod_mpmm_core_ctx core_ctx_table;

    core_ctx_table.delta = &delta;

    gear = mpmm_core_threshold_policy(&dev_ctx_table[0], &core_ctx_table);
    TEST_ASSERT_EQUAL(
        (dev_ctx_table[0].domain_config->num_threshold_counters - 1), gear);
}

void utest_find_perf_limit_from_pct_default_limit(void)
{
    uint32_t perf_limit;
    uint32_t threshold_map =
        (fake_pct_table[FAKE_PCT_TABLE_NO_THRESHOLD_BITMAP]
             .threshold_perf->threshold_bitmap +
         1);

    perf_limit = find_perf_limit_from_pct(
        &fake_pct_table[FAKE_PCT_TABLE_NO_THRESHOLD_BITMAP], threshold_map);
    TEST_ASSERT_EQUAL(
        fake_pct_table[FAKE_PCT_TABLE_NO_THRESHOLD_BITMAP].default_perf_limit,
        perf_limit);
}

void utest_mpmm_evaluate_perf_limit_no_entry(void)
{
    uint32_t perf_limit;

    mpmm_ctx.domain_ctx->num_cores_online = 2;
    dev_ctx_table[0].threshold_map = 0;

    perf_limit = mpmm_evaluate_perf_limit(mpmm_ctx.domain_ctx);
    TEST_ASSERT_EQUAL(
        fake_pct_table[FAKE_PCT_TABLE_DEFAULT].threshold_perf->perf_limit,
        perf_limit);
}

void utest_mpmm_monitor_and_control_no_cores_online(void)
{
    uint32_t prev_perf_limit = mpmm_ctx.domain_ctx->perf_limit;

    mpmm_ctx.domain_ctx->num_cores_online = 0;

    mpmm_monitor_and_control(mpmm_ctx.domain_ctx);
    TEST_ASSERT_EQUAL(prev_perf_limit, mpmm_ctx.domain_ctx->perf_limit);
}

void utest_mpmm_core_counters_delta_read_two_counter(void)
{
    /* Initialize cached_counter to a random value to check delta calculation */
    uint64_t cached_counters[2] = { 0x1111, 0x2222 };
    uint64_t delta[2] = { 0 };
    struct mod_mpmm_core_ctx core_ctx_table;

    core_ctx_table.base_aux_counter_id =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_AMU_MMAP, CORE0_IDX, AMU_AUX0);
    core_ctx_table.cached_counters = cached_counters;
    core_ctx_table.delta = delta;
    mpmm_ctx.amu_driver_api->get_counters = &amu_mmap_copy_data;
    dev_ctx_table[0].domain_config =
        &fake_dom_conf[MPMM_DOM_TWO_THRESHOLD_COUNTER];

    mpmm_core_counters_delta(&dev_ctx_table[0], &core_ctx_table);

    TEST_ASSERT_EQUAL(
        fake_amu_counter[AMU_AUX0], core_ctx_table.cached_counters[0]);
    TEST_ASSERT_EQUAL(
        (fake_amu_counter[AMU_AUX0] - 0x1111), core_ctx_table.delta[0]);

    TEST_ASSERT_EQUAL(
        fake_amu_counter[AMU_AUX1], core_ctx_table.cached_counters[1]);
    TEST_ASSERT_EQUAL(
        (fake_amu_counter[AMU_AUX1] - 0x2222), core_ctx_table.delta[1]);
}

void utest_mpmm_core_counters_delta_wraparound(void)
{
    /* Initialize cached_counter to value close to max to trigger wraparound */
    uint64_t cached_counters = UINT64_MAX - 5;
    uint64_t delta = 0;
    struct mod_mpmm_core_ctx core_ctx_table;

    core_ctx_table.base_aux_counter_id =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_AMU_MMAP, CORE0_IDX, AMU_AUX0);
    core_ctx_table.cached_counters = &cached_counters;
    core_ctx_table.delta = &delta;
    mpmm_ctx.amu_driver_api->get_counters = &amu_mmap_copy_data;

    mpmm_core_counters_delta(&dev_ctx_table[0], &core_ctx_table);
    TEST_ASSERT_EQUAL(
        fake_amu_counter[AMU_AUX0], *core_ctx_table.cached_counters);
    TEST_ASSERT_EQUAL(fake_amu_counter[AMU_AUX0] + 5, *core_ctx_table.delta);
}

void utest_mpmm_core_counters_delta_read_fail(void)
{
    uint64_t cached_counters = UINT64_MAX;
    uint64_t delta = UINT64_MAX;
    struct mod_mpmm_core_ctx core_ctx_table;

    core_ctx_table.base_aux_counter_id =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_AMU_MMAP, CORE0_IDX, AMU_AUX0);
    core_ctx_table.cached_counters = &cached_counters;
    core_ctx_table.delta = &delta;
    mpmm_ctx.amu_driver_api->get_counters = &amu_mmap_return_error;

    mpmm_core_counters_delta(&dev_ctx_table[0], &core_ctx_table);
    /* cached_counters and delta should remain the same if read fail */
    TEST_ASSERT_EQUAL(UINT64_MAX, *core_ctx_table.cached_counters);
    TEST_ASSERT_EQUAL(UINT64_MAX, *core_ctx_table.delta);
}

int mod_mpmm_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_mpmm_start_mod_id_success);
    RUN_TEST(utest_mpmm_start_notif_subscribe_success);
    RUN_TEST(utest_mpmm_start_notif_subscribe_pre_transition_fail);
    RUN_TEST(utest_mpmm_start_notif_subscribe_post_transition_fail);

    RUN_TEST(utest_mpmm_init_success);
    RUN_TEST(utest_mpmm_init_element_count_fail);

    RUN_TEST(utest_mpmm_element_init_two_core_success);
    RUN_TEST(utest_mpmm_element_init_element_count_0_fail);
    RUN_TEST(utest_mpmm_element_init_element_count_max_fail);
    RUN_TEST(utest_mpmm_element_init_max_threshold_count_fail);
    RUN_TEST(utest_mpmm_element_init_num_threshold_mismatch);

    RUN_TEST(utest_mpmm_bind_first_round_success);
    RUN_TEST(utest_mpmm_bind_invalid_module_success);
    RUN_TEST(utest_mpmm_bind_module_bind_success);
    RUN_TEST(utest_mpmm_bind_amu_bind_fail);

    RUN_TEST(utest_mpmm_process_bind_request_success);
    RUN_TEST(utest_mpmm_process_bind_request_id_not_equal);

    RUN_TEST(utest_mpmm_process_notification_pre_state_to_on);
    RUN_TEST(utest_mpmm_process_notification_post_state_to_off);
    RUN_TEST(utest_mpmm_process_notification_core_idx_larger);
    RUN_TEST(utest_mpmm_process_notification_no_perf_change);

    RUN_TEST(utest_mpmm_update_success);
    RUN_TEST(utest_mpmm_update_domain_idx_not_found);
    RUN_TEST(utest_mpmm_update_no_cores_online);
    RUN_TEST(utest_mpmm_update_wait_for_perf_transition);

    RUN_TEST(utest_mpmm_report_success);
    RUN_TEST(utest_mpmm_report_domain_idx_not_found);
    RUN_TEST(utest_mpmm_report_no_wait_for_perf_transition);
    RUN_TEST(utest_mpmm_report_pd_blocked_delayed_resp_fail);
    RUN_TEST(utest_mpmm_report_pd_blocked_put_event_fail);

    RUN_TEST(utest_mpmm_core_evaluate_threshold_success);
    RUN_TEST(utest_mpmm_core_evaluate_threshold_counter_not_enabled);

    RUN_TEST(utest_mpmm_domain_set_thresholds_success);

    RUN_TEST(utest_get_domain_ctx_null);

    RUN_TEST(utest_mpmm_core_threshold_policy_highest_gear);

    RUN_TEST(utest_find_perf_limit_from_pct_default_limit);

    RUN_TEST(utest_mpmm_evaluate_perf_limit_no_entry);

    RUN_TEST(utest_mpmm_monitor_and_control_no_cores_online);

    RUN_TEST(utest_mpmm_core_counters_delta_read_two_counter);
    RUN_TEST(utest_mpmm_core_counters_delta_wraparound);
    RUN_TEST(utest_mpmm_core_counters_delta_read_fail);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return mod_mpmm_test_main();
}
#endif
