/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockmod_power_allocator_extra.h>
#include <Mockmod_power_coordinator_extra.h>
#include <Mockmod_transport_extra.h>
#include <internal/Mockfwk_core_internal.h>

#include <mod_scmi_power_capping_unit_test.h>

#include <stdarg.h>

#include UNIT_TEST_SRC

static int status;

static struct pcapping_fast_channel_ctx fch_ctx_table
    [FAKE_POWER_CAPPING_IDX_COUNT * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT];

static struct mod_scmi_power_capping_domain_context
    power_capping_ctx_table[FAKE_POWER_CAPPING_IDX_COUNT];

static const struct scmi_pcapping_fch_config fch_config[MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT] =
    {
        [MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET] = {
            .fch_support = true,
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FAST_CHANNELS),
        },
        [MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET] = {
            .fch_support = true,
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FAST_CHANNELS),
        },
        [MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET] = {
            .fch_support = true,
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FAST_CHANNELS),
        },
        [MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET] = {
            .fch_support = true,
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FAST_CHANNELS),
        },
    };

static const struct mod_scmi_power_capping_domain_config
    scmi_power_capping_default_config = {
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS
        .parent_idx = __LINE__,
        .min_power_cap = MIN_DEFAULT_POWER_CAP,
        .max_power_cap = MAX_DEFAULT_POWER_CAP,
        .power_cap_step = 1,
#endif
        .fch_config = fch_config,
        .power_allocator_domain_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_ALLOCATOR, __LINE__),
        .power_coordinator_domain_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_COORDINATOR, __LINE__),
    };

static const struct mod_transport_fast_channels_api transport_fch_api = {
    .transport_get_fch_address = transport_get_fch_address,
    .transport_get_fch_interrupt_type = transport_get_fch_interrupt_type,
    .transport_get_fch_doorbell_info = transport_get_fch_doorbell_info,
    .transport_get_fch_rate_limit = transport_get_fch_rate_limit,
    .transport_fch_register_callback = transport_fch_register_callback,
};

static const struct mod_power_allocator_api power_allocator_api = {
    .get_cap = get_cap,
    .set_cap = set_cap,
};

static const struct mod_power_coordinator_api power_coordinator_api = {
    .get_coordinator_period = get_coordinator_period,
    .set_coordinator_period = set_coordinator_period,
};

static const struct mod_scmi_power_capping_power_apis power_management_apis = {
    .power_allocator_api = &power_allocator_api,
    .power_coordinator_api = &power_coordinator_api,
};

static uint32_t local_fast_channel_memory_emulation;
static uint32_t target_fast_channel_memory_emulation;

/* Test functions */
/* Initialise the tests */

void setUp(void)
{
    uint32_t domain_idx, fch_idx, cmd_handler_index;
    struct pcapping_fast_channel_ctx *fch_ctx;

    status = FWK_E_STATE;

    pcapping_fast_channel_global_ctx.fch_count =
        FAKE_POWER_CAPPING_IDX_COUNT * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
    pcapping_fast_channel_global_ctx.fch_ctx_table = fch_ctx_table;

    pcapping_fast_channel_global_ctx.power_management_apis =
        &power_management_apis;

    pcapping_fast_channel_global_ctx.power_capping_domain_ctx_table =
        power_capping_ctx_table;

    for (domain_idx = 0; domain_idx < FAKE_POWER_CAPPING_IDX_COUNT;
         domain_idx++) {
        fch_idx = domain_idx * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
        power_capping_ctx_table[domain_idx].config =
            &scmi_power_capping_default_config;

        for (cmd_handler_index = 0;
             cmd_handler_index < MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
             cmd_handler_index++) {
            fch_ctx = &(pcapping_fast_channel_global_ctx
                            .fch_ctx_table[fch_idx + cmd_handler_index]);
            fch_ctx->fch_config = &(scmi_power_capping_default_config
                                        .fch_config[cmd_handler_index]);
            fch_ctx->fch_address = (struct mod_transport_fast_channel_addr){
                .local_view_address =
                    (uintptr_t)&local_fast_channel_memory_emulation,
                .target_view_address =
                    (uintptr_t)&target_fast_channel_memory_emulation,
            };
            fch_ctx->transport_fch_api = &transport_fch_api;
        }
    }
}

void tearDown(void)
{
}

void utest_pcapping_fast_channel_callback(void)
{
    uintptr_t param = (uintptr_t)NULL;

    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    pcapping_fast_channel_callback(param);
}

void utest_pcapping_fast_channel_process_command_cap_get(void)
{
    uint32_t fch_idx;
    uint32_t cap = __LINE__;

    fch_idx =
        (FAKE_POWER_CAPPING_IDX_1 * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT) +
        MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET;

    get_cap_ExpectWithArrayAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        &cap,
        sizeof(cap),
        FWK_SUCCESS);
    get_cap_IgnoreArg_cap();
    get_cap_ReturnMemThruPtr_cap(&cap, sizeof(cap));

    pcapping_fast_channel_process_command(fch_idx);
    TEST_ASSERT_EQUAL(local_fast_channel_memory_emulation, cap);
}

void utest_pcapping_fast_channel_process_command_cap_set(void)
{
    uint32_t fch_idx;

    fch_idx =
        (FAKE_POWER_CAPPING_IDX_1 * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT) +
        MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET;

    local_fast_channel_memory_emulation = __LINE__;

    set_cap_ExpectAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        local_fast_channel_memory_emulation,
        FWK_SUCCESS);
    pcapping_fast_channel_process_command(fch_idx);
}

void utest_pcapping_fast_channel_process_command_pai_get(void)
{
    uint32_t fch_idx;
    uint32_t pai = __LINE__;

    fch_idx =
        (FAKE_POWER_CAPPING_IDX_1 * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT) +
        MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET;

    get_coordinator_period_ExpectWithArrayAndReturn(
        scmi_power_capping_default_config.power_coordinator_domain_id,
        &pai,
        sizeof(pai),
        FWK_SUCCESS);
    get_coordinator_period_IgnoreArg_period();
    get_coordinator_period_ReturnMemThruPtr_period(&pai, sizeof(pai));

    pcapping_fast_channel_process_command(fch_idx);
    TEST_ASSERT_EQUAL(local_fast_channel_memory_emulation, pai);
}

void utest_pcapping_fast_channel_process_command_pai_set(void)
{
    uint32_t fch_idx;

    fch_idx =
        (FAKE_POWER_CAPPING_IDX_1 * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT) +
        MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET;

    local_fast_channel_memory_emulation = __LINE__;

    set_coordinator_period_ExpectAndReturn(
        scmi_power_capping_default_config.power_coordinator_domain_id,
        local_fast_channel_memory_emulation,
        FWK_SUCCESS);
    pcapping_fast_channel_process_command(fch_idx);
}

void utest_pcapping_fast_channel_process_event_hw_interrupt(void)
{
    struct fwk_event event;

    event = (struct fwk_event){
        .params[0] = MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET,
    };
    pcapping_fast_channel_global_ctx.interrupt_type =
        MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW;

    get_cap_ExpectWithArrayAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        NULL,
        0,
        FWK_SUCCESS);
    get_cap_IgnoreArg_cap();

    status = pcapping_fast_channel_process_event(&event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_fast_channel_process_event_timer_interrupt(void)
{
    uint32_t fch_idx, domain_idx;
    struct fwk_event event;
    enum mod_scmi_power_capping_fast_channels_cmd_handler_index
        cmd_handler_index;

    event = (struct fwk_event){};
    pcapping_fast_channel_global_ctx.interrupt_type =
        MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER;
    for (fch_idx = 0; fch_idx < pcapping_fast_channel_global_ctx.fch_count;
         fch_idx++) {
        domain_idx = fch_idx / (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
        cmd_handler_index =
            fch_idx - (domain_idx * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT);
        switch (cmd_handler_index) {
        case MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET:
            get_cap_ExpectWithArrayAndReturn(
                scmi_power_capping_default_config.power_allocator_domain_id,
                NULL,
                0,
                FWK_SUCCESS);
            get_cap_IgnoreArg_cap();
            break;
        case MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET:
            set_cap_ExpectAndReturn(
                scmi_power_capping_default_config.power_allocator_domain_id,
                local_fast_channel_memory_emulation,
                FWK_SUCCESS);
            break;
        case MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET:
            get_coordinator_period_ExpectWithArrayAndReturn(
                scmi_power_capping_default_config.power_coordinator_domain_id,
                NULL,
                0,
                FWK_SUCCESS);
            get_coordinator_period_IgnoreArg_period();
            break;
        case MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET:
            set_coordinator_period_ExpectAndReturn(
                scmi_power_capping_default_config.power_coordinator_domain_id,
                local_fast_channel_memory_emulation,
                FWK_SUCCESS);
            break;
        default:
            break;
        }
    }
    status = pcapping_fast_channel_process_event(&event);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_fast_channel_ctx_init(void)
{
    uint32_t fast_channel_count;
    struct mod_scmi_power_capping_context ctx;

    ctx.domain_count = FAKE_POWER_CAPPING_IDX_COUNT;

    fast_channel_count =
        FAKE_POWER_CAPPING_IDX_COUNT * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
    fwk_mm_calloc_ExpectAndReturn(
        fast_channel_count,
        sizeof(struct pcapping_fast_channel_ctx),
        &(pcapping_fast_channel_global_ctx.fch_ctx_table));

    pcapping_fast_channel_ctx_init(&ctx);
    TEST_ASSERT_EQUAL(
        pcapping_fast_channel_global_ctx.fch_count, fast_channel_count);
}

void utest_pcapping_fast_channel_set_domain_config(void)
{
    uint32_t domain_idx, fch_idx, cmd_handler_index;
    const struct scmi_pcapping_fch_config *fch_config;

    domain_idx = FAKE_POWER_CAPPING_IDX_1;
    fch_idx = domain_idx * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;

    /* Clear configuration */
    for (cmd_handler_index = 0;
         cmd_handler_index < MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
         cmd_handler_index++) {
        pcapping_fast_channel_global_ctx
            .fch_ctx_table[fch_idx + cmd_handler_index]
            .fch_config = NULL;
    }

    pcapping_fast_channel_set_domain_config(
        domain_idx, &scmi_power_capping_default_config);
    for (cmd_handler_index = 0;
         cmd_handler_index < MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
         cmd_handler_index++) {
        fch_config = pcapping_fast_channel_global_ctx
                         .fch_ctx_table[fch_idx + cmd_handler_index]
                         .fch_config;

        TEST_ASSERT_EQUAL(
            fch_config,
            &(scmi_power_capping_default_config.fch_config[cmd_handler_index]));
    }
}

void utest_pcapping_fast_channel_bind(void)
{
    uint32_t fch_idx;
    struct pcapping_fast_channel_ctx *fch_ctx;

    for (fch_idx = 0; fch_idx < pcapping_fast_channel_global_ctx.fch_count;
         fch_idx++) {
        fch_ctx = &(pcapping_fast_channel_global_ctx.fch_ctx_table[fch_idx]);
        fwk_module_bind_ExpectAndReturn(
            fch_ctx->fch_config->transport_id,
            fch_ctx->fch_config->transport_api_id,
            &(fch_ctx->transport_fch_api),
            FWK_SUCCESS);
    }

    status = pcapping_fast_channel_bind();
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_fast_channel_set_handler(void)
{
    const struct mod_scmi_power_capping_power_apis power_management_api = { 0 };

    pcapping_fast_channel_set_power_apis(&power_management_api);
    TEST_ASSERT_EQUAL(
        pcapping_fast_channel_global_ctx.power_management_apis,
        &power_management_api);
}

void utest_pcapping_fast_channel_start(void)
{
    uint32_t fch_idx;
    const struct scmi_pcapping_fch_config *fch_config;
    struct pcapping_fast_channel_ctx *fch_ctx;

    for (fch_idx = 0; fch_idx < pcapping_fast_channel_global_ctx.fch_count;
         fch_idx++) {
        fch_ctx = &(pcapping_fast_channel_global_ctx.fch_ctx_table[fch_idx]);
        fch_config = fch_ctx->fch_config;
        transport_get_fch_address_ExpectAndReturn(
            fch_config->transport_id, &(fch_ctx->fch_address), FWK_SUCCESS);
        transport_get_fch_rate_limit_ExpectAndReturn(
            fch_config->transport_id, &(fch_ctx->fch_rate_limit), FWK_SUCCESS);
        transport_get_fch_interrupt_type_ExpectAndReturn(
            fch_config->transport_id,
            &(pcapping_fast_channel_global_ctx.interrupt_type),
            FWK_SUCCESS);
        transport_fch_register_callback_ExpectAndReturn(
            fch_config->transport_id,
            (uintptr_t)fch_idx,
            pcapping_fast_channel_callback,
            FWK_SUCCESS);
    }

    pcapping_fast_channel_start();
}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(utest_pcapping_fast_channel_callback);
    RUN_TEST(utest_pcapping_fast_channel_process_command_cap_get);
    RUN_TEST(utest_pcapping_fast_channel_process_command_cap_set);
    RUN_TEST(utest_pcapping_fast_channel_process_command_pai_get);
    RUN_TEST(utest_pcapping_fast_channel_process_command_pai_set);
    RUN_TEST(utest_pcapping_fast_channel_process_event_hw_interrupt);
    RUN_TEST(utest_pcapping_fast_channel_process_event_timer_interrupt);
    RUN_TEST(utest_pcapping_fast_channel_ctx_init);
    RUN_TEST(utest_pcapping_fast_channel_set_domain_config);
    RUN_TEST(utest_pcapping_fast_channel_bind);
    RUN_TEST(utest_pcapping_fast_channel_set_handler);
    RUN_TEST(utest_pcapping_fast_channel_start);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif
