/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_power_domain.h>
#include <mod_synquacer_system.h>
#include <mod_system_power.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_multi_thread.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>

struct synquacer_system_ctx synquacer_system_ctx;
const struct fwk_module_config config_synquacer_system = { 0 };

enum synquacer_system_event {
    SYNQUACER_SYSTEM_EVENT_START,
    SYNQUACER_SYSTEM_EVENT_COUNT
};

int reboot_chip(void);
void power_domain_reboot(void);
void main_initialize(void);
int synquacer_main(void);

/*
 * SYSTEM POWER driver API
 */

static int synquacer_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
    FWK_LOG_INFO("[SYNQUACER SYSTEM] requesting synquacer system_shutdown");

    reboot_chip();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    synquacer_system_power_driver_api = {
        .system_shutdown = synquacer_system_shutdown,
    };

/*
 * Functions fulfilling the framework's module interface
 */

static int synquacer_system_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *unused2)
{
    return FWK_SUCCESS;
}

static int synquacer_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 0) {
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_CCN512),
            FWK_ID_API(FWK_MODULE_IDX_CCN512, 0),
            &synquacer_system_ctx.ccn512_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_F_I2C),
            FWK_ID_API(FWK_MODULE_IDX_F_I2C, 0),
            &synquacer_system_ctx.f_i2c_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_HSSPI),
            FWK_ID_API(FWK_MODULE_IDX_HSSPI, 0),
            &synquacer_system_ctx.hsspi_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
            FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
            &synquacer_system_ctx.mod_pd_restricted_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int synquacer_system_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &synquacer_system_power_driver_api;
    return FWK_SUCCESS;
}

static int synquacer_system_start(fwk_id_t id)
{
    int status;
    struct fwk_event req;

    main_initialize();

    FWK_LOG_INFO("[SYNQUACER SYSTEM] Request system initialization.");

    status = fwk_thread_create(FWK_ID_MODULE(FWK_MODULE_IDX_SYNQUACER_SYSTEM));
    if (status != FWK_SUCCESS)
        return status;

    req = (struct fwk_event){
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_SYNQUACER_SYSTEM, SYNQUACER_SYSTEM_EVENT_START),
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_SYNQUACER_SYSTEM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SYNQUACER_SYSTEM),
        .response_requested = false,
    };

    status = fwk_thread_put_event(&req);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

int synquacer_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    if (fwk_id_get_event_idx(event->id) == SYNQUACER_SYSTEM_EVENT_START) {
        FWK_LOG_INFO("[SYNQUACER SYSTEM] Process system start event.");
        synquacer_main();
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_synquacer_system = {
    .name = "SYNQUACER_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .event_count = 1,
    .init = synquacer_system_mod_init,
    .bind = synquacer_system_bind,
    .process_bind_request = synquacer_system_process_bind_request,
    .start = synquacer_system_start,
    .process_event = synquacer_process_event,
};
