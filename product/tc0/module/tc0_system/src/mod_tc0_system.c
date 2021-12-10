/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     TC0 System Support.
 */

#include "clock_soc.h"
#include "scp_pik.h"
#include "tc0_core.h"
#include "tc0_scmi.h"
#include "tc0_sds.h"

#include <mod_clock.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_power.h>
#include <mod_tc0_system.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>

#include <fmw_cmsis.h>

#include <stdint.h>

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    SCP_TC0_SCMI_SERVICE_IDX_PSCI,
};

/* Module context */
struct tc0_system_ctx {
    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* SDS API pointer */
    const struct mod_sds_api *sds_api;
};

struct tc0_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct tc0_system_ctx tc0_system_ctx;
const struct fwk_module_config config_tc0_system = { 0 };

static const uint32_t feature_flags =
    (TC0_SDS_FEATURE_FIRMWARE_MASK | TC0_SDS_FEATURE_DMC_MASK |
     TC0_SDS_FEATURE_MESSAGING_MASK);

static fwk_id_t sds_feature_availability_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, 1);

/*
 *  SCMI Messaging stack
 */

static int messaging_stack_ready(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_feature_availability_id);

    /*
     * Write SDS Feature Availability to signal the completion of the messaging
     * stack
     */
    return tc0_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&feature_flags),
        sds_structure_desc->size);
}

/*
 * System power's driver API
 */

static int tc0_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    tc0_system_system_power_driver_api = {
        .system_shutdown = tc0_system_shutdown,
    };

/*
 * Functions fulfilling the framework's module interface
 */

static int tc0_system_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *unused2)
{
    return FWK_SUCCESS;
}

static int tc0_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &tc0_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &tc0_system_ctx.ppu_v1_isr_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        fwk_module_id_sds,
        FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
        &tc0_system_ctx.sds_api);
}

static int tc0_system_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &tc0_system_system_power_driver_api;
    return FWK_SUCCESS;
}

static int tc0_system_start(fwk_id_t id)
{
    int status;
    unsigned int i;

    /*
     * Subscribe to these SCMI channels in order to know when they have all
     * initialized.
     * At that point we can consider the SCMI stack to be initialized from
     * the point of view of the PSCI agent.
     */
    for (i = 0; i < FWK_ARRAY_SIZE(scmi_notification_table); i++) {
        status = fwk_notification_subscribe(
            mod_scmi_notification_id_initialized,
            fwk_id_build_element_id(
                fwk_module_id_scmi, scmi_notification_table[i]),
            id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /*
     * Subscribe to the SDS initialized notification so we can correctly let the
     * PSCI agent know that the SCMI stack is initialized.
     */
    status = fwk_notification_subscribe(
        mod_sds_notification_id_initialized, fwk_module_id_sds, id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return tc0_system_ctx.mod_pd_restricted_api->set_state(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
        MOD_PD_SET_STATE_NO_RESP,
        MOD_PD_COMPOSITE_STATE(
            MOD_PD_LEVEL_2,
            0,
            MOD_PD_STATE_ON,
            MOD_PD_STATE_OFF,
            MOD_PD_STATE_OFF));
}

static int tc0_system_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    if (fwk_id_is_equal(event->id, mod_scmi_notification_id_initialized)) {
        scmi_notification_count++;
    } else if (fwk_id_is_equal(
                   event->id, mod_sds_notification_id_initialized)) {
        sds_notification_received = true;
    } else {
        return FWK_E_PARAM;
    }

    if ((scmi_notification_count == FWK_ARRAY_SIZE(scmi_notification_table)) &&
        sds_notification_received) {
        messaging_stack_ready();

        scmi_notification_count = 0;
        sds_notification_received = false;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_tc0_system = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_TC0_SYSTEM_API_COUNT,
    .init = tc0_system_mod_init,
    .bind = tc0_system_bind,
    .process_bind_request = tc0_system_process_bind_request,
    .process_notification = tc0_system_process_notification,
    .start = tc0_system_start,
};
