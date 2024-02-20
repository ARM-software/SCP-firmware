/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_ppu_v1.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockfwk_notification.h>
#include <Mockmod_ppu_v1_extra.h>
#include <internal/Mockfwk_core_internal.h>

#include <mod_ppu_v1_extra.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_notification.h>
#include UNIT_TEST_SRC

#define CORES_PER_CLUSTER 2
#define PD_COUNT          2

static struct ppu_v1_pd_ctx pd_table[PD_COUNT];

static struct mod_timer_alarm_api alarm_api_driver = {
    .start = start_alarm_api,
    .stop = stop_alarm_api,
};

void setUp(void)
{
    memset(&ppu_v1_ctx, 0, sizeof(ppu_v1_ctx));
    ppu_v1_ctx.pd_ctx_table_size = PD_COUNT;

    ppu_v1_ctx.pd_ctx_table = pd_table;

    ppu_v1_ctx.max_num_cores_per_cluster =
        ppu_v1_config_data_ut.num_of_cores_in_cluster;

    ppu_v1_ctx.pd_ctx_table->config = pd_ppu_ctx_config;

    for (int i = 0; i < PD_COUNT; i++) {
        ppu_v1_ctx.pd_ctx_table[i].alarm_api = &alarm_api_driver;
    }
}

void tearDown(void)
{
}

void test_ppu_v1_pd_init_error(void)
{
    int status;
    fwk_id_t pd_id;
    unsigned int unused = 0;
    struct mod_ppu_v1_pd_config config;

    config.pd_type = MOD_PD_TYPE_COUNT + 1;
    status = ppu_v1_pd_init(pd_id, unused, &config);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

void test_ppu_v1_pd_init(void)
{
    int status;
    fwk_id_t pd_id;
    unsigned int unused = 0;
    struct mod_ppu_v1_pd_config config;

    config.pd_type = MOD_PD_TYPE_CLUSTER;
    config.timer_config = NULL;
    config.ppu.irq = FWK_INTERRUPT_NONE;
    config.default_power_on = false;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_optional_id_is_defined_ExpectAnyArgsAndReturn(true);

    struct ppu_v1_pd_ctx *core_pd_ctx_table_temp[CORES_PER_CLUSTER];
    static struct ppu_v1_pd_ctx p0;
    static struct ppu_v1_pd_ctx p1;

    core_pd_ctx_table_temp[0] = &p0;
    core_pd_ctx_table_temp[1] = &p1;

    /* Make a local one to get the size for the next malloc */
    struct ppu_v1_cluster_pd_ctx cluster_pd_ctx_temp;

    cluster_pd_ctx_temp.core_pd_ctx_table =
        (struct ppu_v1_pd_ctx **)&core_pd_ctx_table_temp;
    cluster_pd_ctx_temp.core_count = CORES_PER_CLUSTER;

    fwk_mm_calloc_ExpectAndReturn(
        1, sizeof(cluster_pd_ctx_temp), &ppu_v1_ctx.pd_ctx_table);

    fwk_mm_calloc_ExpectAndReturn(
        ppu_v1_ctx.max_num_cores_per_cluster,
        sizeof(core_pd_ctx_table_temp[0]),
        &core_pd_ctx_table_temp);

    status = ppu_v1_pd_init(pd_id, unused, &config);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_ppu_v1_mod_init(void)
{
    fwk_id_t mod_id;
    int status;

    fwk_mm_calloc_ExpectAndReturn(
        PD_COUNT, sizeof(struct ppu_v1_pd_ctx), &pd_table);

    /* Clear to ensure it gets reset */
    ppu_v1_ctx.pd_ctx_table_size = 0;
    ppu_v1_ctx.max_num_cores_per_cluster = 0;

    fwk_id_build_module_id_ExpectAnyArgsAndReturn(mod_id);

    status = ppu_v1_mod_init(mod_id, PD_COUNT, &ppu_v1_config_data_ut);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(ppu_v1_ctx.pd_ctx_table_size, PD_COUNT);
    TEST_ASSERT_EQUAL(ppu_v1_ctx.max_num_cores_per_cluster, CORES_PER_CLUSTER);
}

void test_ppu_v1_core_pd_set_state_sleep(void)
{
    int status;
    fwk_id_t core_pd_id;
    struct ppu_v1_pd_ctx *pd_ctx_temp;

    pd_ctx_temp = &ppu_v1_ctx.pd_ctx_table[0];

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    ppu_v1_is_dynamic_enabled_ExpectAnyArgsAndReturn(false);
    ppu_v1_dynamic_enable_ExpectAnyArgs();
    ppu_v1_set_input_edge_sensitivity_Expect(
        pd_ctx_temp->ppu, PPU_V1_MODE_ON, PPU_V1_EDGE_SENSITIVITY_MASKED);
    ppu_v1_lock_off_enable_ExpectAnyArgs();
    ppu_v1_interrupt_unmask_ExpectAnyArgs();
    ppu_v1_set_input_edge_sensitivity_Expect(
        pd_ctx_temp->ppu, PPU_V1_MODE_ON, PPU_V1_EDGE_SENSITIVITY_MASKED);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    start_alarm_api_ExpectAndReturn(
        ppu_v1_ctx.pd_ctx_table[0].config->alarm_id,
        ppu_v1_ctx.pd_ctx_table[0].config->alarm_delay,
        MOD_TIMER_ALARM_TYPE_ONCE,
        deeper_locking_alarm_callback,
        0,
        FWK_SUCCESS);
    status = ppu_v1_core_pd_set_state(core_pd_id, MOD_PD_STATE_SLEEP);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_start_deeper_locking_alarm(void)
{
    int status;
    fwk_id_t core_pd_id;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    start_alarm_api_ExpectAndReturn(
        ppu_v1_ctx.pd_ctx_table[0].config->alarm_id,
        ppu_v1_ctx.pd_ctx_table[0].config->alarm_delay,
        MOD_TIMER_ALARM_TYPE_ONCE,
        deeper_locking_alarm_callback,
        0,
        FWK_SUCCESS);

    status = start_deeper_locking_alarm(core_pd_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_start_deeper_locking_alarm_null_api(void)
{
    int status;
    fwk_id_t core_pd_id;

    ppu_v1_ctx.pd_ctx_table[0].alarm_api = NULL;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = start_deeper_locking_alarm(core_pd_id);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_deeper_locking_alarm_callback(void)
{
    uintptr_t param = (uintptr_t)0;
    struct ppu_v1_pd_ctx *pd_ctx_temp;
    pd_ctx_temp = &ppu_v1_ctx.pd_ctx_table[0];

    ppu_v1_lock_off_disable_Expect(pd_ctx_temp->ppu);
    ppu_v1_off_unlock_ExpectAnyArgs();

    deeper_locking_alarm_callback(param);
}

int scmi_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_ppu_v1_mod_init);
    RUN_TEST(test_ppu_v1_pd_init_error);
    RUN_TEST(test_ppu_v1_pd_init);
    RUN_TEST(test_ppu_v1_core_pd_set_state_sleep);
    RUN_TEST(test_start_deeper_locking_alarm);
    RUN_TEST(test_start_deeper_locking_alarm_null_api);
    RUN_TEST(test_deeper_locking_alarm_callback);

    return UNITY_END();
}

int main(void)
{
    return scmi_test_main();
}
