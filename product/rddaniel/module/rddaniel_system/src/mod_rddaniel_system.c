/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RD-Daniel System Support.
 */

#include "clock_soc.h"
#include "rddaniel_core.h"
#include "rddaniel_scmi.h"
#include "rddaniel_sds.h"
#include "scp_css_mmap.h"
#include "scp_pik.h"
#include "scp_rddaniel_irq.h"

#include <mod_clock.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_rddaniel_system.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    SCP_RDDANIEL_SCMI_SERVICE_IDX_PSCI,
};

/* Module context */
struct rddaniel_system_ctx {
    /* Pointer to the SCP PIK registers */
    struct pik_scp_reg *pik_scp_reg;

    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* SDS API pointer */
    const struct mod_sds_api *sds_api;
};

struct rddaniel_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct rddaniel_system_ctx rddaniel_system_ctx;
const struct fwk_module_config config_rddaniel_system = { 0 };

static const uint32_t feature_flags = (RDDANIEL_SDS_FEATURE_FIRMWARE_MASK |
                                       RDDANIEL_SDS_FEATURE_DMC_MASK |
                                       RDDANIEL_SDS_FEATURE_MESSAGING_MASK);

static fwk_id_t sds_feature_availability_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, 3);

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
    return rddaniel_system_ctx.sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&feature_flags), sds_structure_desc->size);
}

/*
 *  PPU Interrupt Service Routines for cluster and core power domains
 */

static void ppu_cores_isr(unsigned int first, uint32_t status)
{
    unsigned int core_idx;

    while (status != 0) {
        core_idx = __builtin_ctz(status);
        status &= ~(1 << core_idx);

        if ((first + core_idx) >= rddaniel_core_get_core_count())
            continue;

        rddaniel_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, first + core_idx));
    }
}

static void ppu_cores_isr_0(void)
{
    ppu_cores_isr(0, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[0]);
    ppu_cores_isr(128, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[4]);
}

static void ppu_cores_isr_1(void)
{
    ppu_cores_isr(32, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[1]);
    ppu_cores_isr(160, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[5]);
}

static void ppu_cores_isr_2(void)
{
    ppu_cores_isr(64, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[2]);
    ppu_cores_isr(192, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[6]);
}

static void ppu_cores_isr_3(void)
{
    ppu_cores_isr(96, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[3]);
    ppu_cores_isr(224, rddaniel_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[7]);
}

static void ppu_clusters_isr(void)
{
    uint32_t status = rddaniel_system_ctx.pik_scp_reg->CLUS_PPU_INT_STATUS;
    unsigned int cluster_idx;

    while (status != 0) {
        cluster_idx = __builtin_ctz(status);

        rddaniel_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1,
            rddaniel_core_get_core_count() + cluster_idx));

        status &= ~(1 << cluster_idx);
    }
}

/*
 *  PPU Interrupt Service Routine table
 */

static struct rddaniel_system_isr isrs[] = {
    [0] = { .interrupt = PPU_CORES0_IRQ,
            .handler = ppu_cores_isr_0 },
    [1] = { .interrupt = PPU_CORES1_IRQ,
            .handler = ppu_cores_isr_1 },
    [2] = { .interrupt = PPU_CORES2_IRQ,
            .handler = ppu_cores_isr_2 },
    [3] = { .interrupt = PPU_CORES3_IRQ,
            .handler = ppu_cores_isr_3 },
    [4] = { .interrupt = PPU_CLUSTERS_IRQ,
            .handler = ppu_clusters_isr },
};

/*
 * System power's driver API
 */

static int rddaniel_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    rddaniel_system_system_power_driver_api = {
    .system_shutdown = rddaniel_system_shutdown,
};

/*
 * Functions fulfilling the framework's module interface
 */

static int rddaniel_system_mod_init(fwk_id_t module_id, unsigned int unused,
    const void *unused2)
{
    int status;
    unsigned int idx;
    struct rddaniel_system_isr *isr;

    for (idx = 0; idx < FWK_ARRAY_SIZE(isrs); idx++) {
        isr = &isrs[idx];
        status = fwk_interrupt_set_isr(isr->interrupt, isr->handler);
        if (status != FWK_SUCCESS)
            return status;
    }

    rddaniel_system_ctx.pik_scp_reg = (struct pik_scp_reg *)SCP_PIK_SCP_BASE;

    return FWK_SUCCESS;
}

static int rddaniel_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &rddaniel_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &rddaniel_system_ctx.ppu_v1_isr_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(fwk_module_id_sds,
        FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
        &rddaniel_system_ctx.sds_api);
}

static int rddaniel_system_process_bind_request(fwk_id_t requester_id,
    fwk_id_t pd_id, fwk_id_t api_id, const void **api)
{
    *api = &rddaniel_system_system_power_driver_api;
    return FWK_SUCCESS;
}

static int rddaniel_system_start(fwk_id_t id)
{
    int status;
    unsigned int i;

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
    if (status != FWK_SUCCESS)
        return status;

    FWK_LOG_INFO("[RD-DANIEL SYSTEM] Requesting SYSTOP initialization...");

    /*
     * Subscribe to these SCMI channels in order to know when they have all
     * initialized.
     * At that point we can consider the SCMI stack to be initialized from
     * the point of view of the PSCI agent.
     */
    for (i = 0; i < FWK_ARRAY_SIZE(scmi_notification_table); i++) {
        status = fwk_notification_subscribe(
            mod_scmi_notification_id_initialized,
            fwk_id_build_element_id(fwk_module_id_scmi,
                scmi_notification_table[i]),
            id);
        if (status != FWK_SUCCESS)
            return status;
    }

    /*
     * Subscribe to the SDS initialized notification so we can correctly let the
     * PSCI agent know that the SCMI stack is initialized.
     */
    status = fwk_notification_subscribe(
        mod_sds_notification_id_initialized,
        fwk_module_id_sds,
        id);
    if (status != FWK_SUCCESS)
        return status;

    return
        rddaniel_system_ctx.mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0), false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                                   MOD_PD_STATE_OFF, MOD_PD_STATE_OFF));
}

int rddaniel_system_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct clock_notification_params *params;
    struct mod_pd_restricted_api *mod_pd_restricted_api;
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;

    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        params = (struct clock_notification_params *)event->params;

        /*
         * Initialize primary core when the system is initialized for the first
         * time only
         */
        if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
            FWK_LOG_INFO("[RD-DANIEL SYSTEM] Initializing the primary core...");

            mod_pd_restricted_api = rddaniel_system_ctx.mod_pd_restricted_api;

            status =  mod_pd_restricted_api->set_composite_state_async(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
                false,
                MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                    MOD_PD_STATE_ON, MOD_PD_STATE_ON));
            if (status != FWK_SUCCESS)
                return status;

            /* Unsubscribe to the notification */
            return fwk_notification_unsubscribe(event->id, event->source_id,
                                                event->target_id);
        }

        return FWK_SUCCESS;
    } else if (fwk_id_is_equal(event->id,
                               mod_scmi_notification_id_initialized)) {
        scmi_notification_count++;
    } else if (fwk_id_is_equal(event->id,
                               mod_sds_notification_id_initialized)) {
        sds_notification_received = true;
    } else
        return FWK_E_PARAM;

    if ((scmi_notification_count == FWK_ARRAY_SIZE(scmi_notification_table)) &&
        sds_notification_received) {
        messaging_stack_ready();

        scmi_notification_count = 0;
        sds_notification_received = false;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_rddaniel_system = {
    .name = "RDDANIEL_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RDDANIEL_SYSTEM_API_COUNT,
    .init = rddaniel_system_mod_init,
    .bind = rddaniel_system_bind,
    .process_bind_request = rddaniel_system_process_bind_request,
    .process_notification = rddaniel_system_process_notification,
    .start = rddaniel_system_start,
};
