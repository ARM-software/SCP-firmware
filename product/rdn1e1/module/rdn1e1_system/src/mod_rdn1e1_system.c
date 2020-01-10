/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RDN1E1 System Support.
 */

#include "config_clock.h"
#include "rdn1e1_core.h"
#include "rdn1e1_pik_scp.h"
#include "rdn1e1_sds.h"
#include "scp_rdn1e1_irq.h"
#include "scp_rdn1e1_mmap.h"
#include "scp_rdn1e1_scmi.h"

#include <mod_clock.h>
#include <mod_cmn600.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_rdn1e1_system.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_info.h>
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
    SCP_RDN1E1_SCMI_SERVICE_IDX_PSCI,
    SCP_RDN1E1_SCMI_SERVICE_IDX_OSPM,
};

/* Module context */
struct rdn1e1_system_ctx {
    /* Pointer to the SCP PIK registers */
    struct pik_scp_reg *pik_scp_reg;

    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* SDS API pointer */
    const struct mod_sds_api *sds_api;

    /* CMN600 CCIX config API pointer */
    struct mod_cmn600_ccix_config_api *cmn600_api;

    /* System Information HAL API pointer */
    struct mod_system_info_get_info_api *system_info_api;
};

struct rdn1e1_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct rdn1e1_system_ctx rdn1e1_system_ctx;
const struct fwk_module_config config_rdn1e1_system = { 0 };

static const uint32_t feature_flags = (RDN1E1_SDS_FEATURE_FIRMWARE_MASK |
                                       RDN1E1_SDS_FEATURE_DMC_MASK |
                                       RDN1E1_SDS_FEATURE_MESSAGING_MASK);

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
    return rdn1e1_system_ctx.sds_api->struct_write(sds_structure_desc->id,
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

        if ((first + core_idx) >= rdn1e1_core_get_core_count())
            continue;

        rdn1e1_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, first + core_idx));
    }
}

static void ppu_cores_isr_0(void)
{
    ppu_cores_isr(0, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[0]);
    ppu_cores_isr(128, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[4]);
}

static void ppu_cores_isr_1(void)
{
    ppu_cores_isr(32, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[1]);
    ppu_cores_isr(160, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[5]);
}

static void ppu_cores_isr_2(void)
{
    ppu_cores_isr(64, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[2]);
    ppu_cores_isr(192, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[6]);
}

static void ppu_cores_isr_3(void)
{
    ppu_cores_isr(96, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[3]);
    ppu_cores_isr(224, rdn1e1_system_ctx.pik_scp_reg->CPU_PPU_INT_STATUS[7]);
}

static void ppu_clusters_isr(void)
{
    uint32_t status = rdn1e1_system_ctx.pik_scp_reg->CLUS_PPU_INT_STATUS;
    unsigned int cluster_idx;

    while (status != 0) {
        cluster_idx = __builtin_ctz(status);

        rdn1e1_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1,
            rdn1e1_core_get_core_count() + cluster_idx));

        status &= ~(1 << cluster_idx);
    }
}

/*
 *  PPU Interrupt Service Routine table
 */

static struct rdn1e1_system_isr isrs[] = {
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

static int rdn1e1_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    rdn1e1_system_system_power_driver_api = {
    .system_shutdown = rdn1e1_system_shutdown,
};

/*
 * Functions fulfilling the framework's module interface
 */

static int rdn1e1_system_mod_init(fwk_id_t module_id, unsigned int unused,
    const void *unused2)
{
    int status;
    unsigned int idx;
    struct rdn1e1_system_isr *isr;

    for (idx = 0; idx < FWK_ARRAY_SIZE(isrs); idx++) {
        isr = &isrs[idx];
        status = fwk_interrupt_set_isr(isr->interrupt, isr->handler);
        if (status != FWK_SUCCESS)
            return status;
    }

    rdn1e1_system_ctx.pik_scp_reg = (struct pik_scp_reg *)SCP_PIK_SCP_BASE;

    return FWK_SUCCESS;
}

static int rdn1e1_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &rdn1e1_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &rdn1e1_system_ctx.ppu_v1_isr_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_CMN600),
                             FWK_ID_API(FWK_MODULE_IDX_CMN600,
                                        MOD_CMN600_API_IDX_CCIX_CONFIG),
                             &rdn1e1_system_ctx.cmn600_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SYSTEM_INFO),
                             FWK_ID_API(FWK_MODULE_IDX_SYSTEM_INFO,
                                        MOD_SYSTEM_INFO_GET_API_IDX),
                             &rdn1e1_system_ctx.system_info_api);
   if (status != FWK_SUCCESS)
       return status;

    return fwk_module_bind(fwk_module_id_sds,
        FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
        &rdn1e1_system_ctx.sds_api);
}

static int rdn1e1_system_process_bind_request(fwk_id_t requester_id,
    fwk_id_t pd_id, fwk_id_t api_id, const void **api)
{
    *api = &rdn1e1_system_system_power_driver_api;
    return FWK_SUCCESS;
}

static int rdn1e1_system_start(fwk_id_t id)
{
    int status;
    unsigned int i;

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
    if (status != FWK_SUCCESS)
        return status;

    FWK_LOG_INFO("[RDN1E1 SYSTEM] Requesting SYSTOP initialization...");

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
        rdn1e1_system_ctx.mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0), false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                                   MOD_PD_STATE_OFF, MOD_PD_STATE_OFF));
}

int rdn1e1_system_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct clock_notification_params *params;
    struct mod_pd_restricted_api *mod_pd_restricted_api;
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;
    struct mod_cmn600_ccix_remote_node_config remote_config;
    const struct mod_system_info *system_info;
    uint8_t chip_id = 0;
    bool mc_mode = false;

    status = rdn1e1_system_ctx.system_info_api->get_system_info(&system_info);
    if (status == FWK_SUCCESS) {
        chip_id = system_info->chip_id;
        mc_mode = system_info->multi_chip_mode;
    }

    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        params = (struct clock_notification_params *)event->params;
        if (params->new_state != MOD_CLOCK_STATE_RUNNING)
            return FWK_SUCCESS;

        /* Perform the CCIX setup if multi-chip mode is detected */
        if (mc_mode == true) {
            /* Populate CCIX config data statically */
            remote_config.remote_rnf_count = 2;
            remote_config.remote_sa_count = 0;
            remote_config.remote_ha_count = 1;
            remote_config.ccix_link_id = 0;
            remote_config.remote_ha_mmap_count = 1;
            remote_config.smp_mode = true;
            if (chip_id == 0) {
                remote_config.remote_ha_mmap[0].ha_id = 0x1;
                remote_config.remote_ha_mmap[0].base = (4ULL * FWK_TIB);
            } else {
                remote_config.remote_ha_mmap[0].ha_id = 0x0;
                remote_config.remote_ha_mmap[0].base = 0x0;
            }
            remote_config.remote_ha_mmap[0].size = (4ULL * FWK_TIB);

            status = rdn1e1_system_ctx.cmn600_api->set_config(&remote_config);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR("CCIX Setup Failed for Chip: %d!", chip_id);
                return status;
            }
            rdn1e1_system_ctx.cmn600_api->exchange_protocol_credit(0);
            rdn1e1_system_ctx.cmn600_api->enter_system_coherency(0);
            rdn1e1_system_ctx.cmn600_api->enter_dvm_domain(0);
        }

        /*
         * Initialize primary core when the system is initialized for the first
         * time only
         */
        if (chip_id == 0) {
            FWK_LOG_INFO("[RDN1E1 SYSTEM] Initializing the primary core...");

            mod_pd_restricted_api = rdn1e1_system_ctx.mod_pd_restricted_api;

            status = mod_pd_restricted_api->set_composite_state_async(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
                false,
                MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                    MOD_PD_STATE_ON, MOD_PD_STATE_ON));
            if (status != FWK_SUCCESS)
                return status;
        } else {
            FWK_LOG_INFO(
                "[RDN1E1 SYSTEM] Detected as slave chip %d, "
                "Waiting for SCMI command",
                chip_id);
        }

        /* Unsubscribe to the notification */
        return fwk_notification_unsubscribe(event->id, event->source_id,
                                            event->target_id);
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

const struct fwk_module module_rdn1e1_system = {
    .name = "RDN1E1_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RDN1E1_SYSTEM_API_COUNT,
    .init = rdn1e1_system_mod_init,
    .bind = rdn1e1_system_bind,
    .process_bind_request = rdn1e1_system_process_bind_request,
    .process_notification = rdn1e1_system_process_notification,
    .start = rdn1e1_system_start,
};
