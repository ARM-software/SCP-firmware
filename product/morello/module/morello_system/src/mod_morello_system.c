/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO System Support.
 */

#include "config_clock.h"
#include "morello_core.h"
#include "morello_pik_cpu.h"
#include "morello_pik_debug.h"
#include "morello_pik_scp.h"
#include "morello_scp_mmap.h"
#include "morello_scp_pik.h"
#include "morello_scp_scmi.h"
#include "morello_sds.h"

#include <mod_clock.h>
#include <mod_fip.h>
#include <mod_morello_system.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * Platform information structure used by BL31
 */
struct morello_platform_info {
    /* If multichip mode */
    bool multichip_mode;
    /* Total number of slave chips  */
    uint8_t slave_count;
    /* Local ddr size in GB */
    uint8_t local_ddr_size;
    /* Remote ddr size in GB */
    uint8_t remote_ddr_size;
};

/*
 * BL33 image information structure used by BL31
 */
struct morello_bl33_info {
    /* Source address of BL33 image */
    uint32_t bl33_src_addr;
    /* Load address of BL33 image */
    uint32_t bl33_dst_addr;
    /* BL33 image size */
    uint32_t bl33_size;
};

/* MultiChip information */
struct morello_multichip_info {
    /* If multichip mode */
    bool mode;
    /* Total number of slave chips  */
    uint8_t slave_count;
    /* Remote ddr size in GB */
    uint8_t remote_ddr_size;
};

/* Coresight counter register definitions */
struct cs_cnt_ctrl_reg {
    FWK_RW uint32_t CS_CNTCR;
    FWK_R uint32_t CS_CNTSR;
    FWK_RW uint32_t CS_CNTCVLW;
    FWK_RW uint32_t CS_CNTCVUP;
};

#define CS_CNTCONTROL ((struct cs_cnt_ctrl_reg *)SCP_CS_CNTCONTROL_BASE)

/* SCMI Services used by software on the AP cores */
static unsigned int scmi_notification_table[] = {
    SCP_MORELLO_SCMI_SERVICE_IDX_PSCI,
    SCP_MORELLO_SCMI_SERVICE_IDX_OSPM,
};

/* SDS Feature flags */
static const uint32_t feature_flags =
    (MORELLO_SDS_FEATURE_FIRMWARE_MASK | MORELLO_SDS_FEATURE_DMC_MASK |
     MORELLO_SDS_FEATURE_MESSAGING_MASK);
static fwk_id_t sds_feature_availability_id = FWK_ID_ELEMENT_INIT(
    FWK_MODULE_IDX_SDS,
    SDS_ELEMENT_IDX_FEATURE_AVAILABILITY);

/* SDS DDR memory information */
static struct morello_platform_info sds_platform_info;
static fwk_id_t sds_platform_info_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, SDS_ELEMENT_IDX_PLATFORM_INFO);

/* SDS BL33 image information */
static struct morello_bl33_info sds_bl33_info;
static fwk_id_t sds_bl33_info_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, SDS_ELEMENT_IDX_BL33_INFO);

/* Module context */
struct morello_system_ctx {
    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* Pointer to FIP APIs */
    const struct mod_fip_api *fip_api;

    /* Pointer to DMC Bing memory information API */
    const struct mod_dmc_bing_mem_info_api *dmc_bing_api;

    /* Pointer to SDS */
    const struct mod_sds_api *sds_api;
};

struct morello_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct morello_system_ctx morello_system_ctx;
const struct fwk_module_config config_morello_system = { 0 };

/*
 *  PPU Interrupt Service Routines for cluster and core power domains
 */

static void ppu_cores_isr(void)
{
    uint32_t status = PIK_SCP->CPU_PPU_INT_STATUS[0];
    unsigned int core_idx;

    while (status != 0) {
        core_idx = __builtin_ctz(status);
        status &= ~(1 << core_idx);

        if (core_idx >= morello_core_get_core_count())
            continue;

        morello_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, core_idx));
    }
}

static void ppu_clusters_isr(void)
{
    uint32_t status = PIK_SCP->CLUS_PPU_INT_STATUS;
    unsigned int cluster_idx;

    while (status != 0) {
        cluster_idx = __builtin_ctz(status);

        morello_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(FWK_ID_ELEMENT(
            FWK_MODULE_IDX_PPU_V1,
            morello_core_get_core_count() + cluster_idx));

        status &= ~(1 << cluster_idx);
    }
}

/*
 *  PPU Interrupt Service Routine table
 */

static struct morello_system_isr isrs[] = {
    [0] = { .interrupt = PPU_CORES0_IRQ, .handler = ppu_cores_isr },
    [1] = { .interrupt = PPU_CLUSTERS_IRQ, .handler = ppu_clusters_isr },
};

/*
 * System power module's driver API
 */

static int morello_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();
    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    morello_system_power_driver_api = {
        .system_shutdown = morello_system_shutdown,
    };

/*
 * AP memory 1MB windowed access driver API
 */

/*
 * Function to set the AP address for which the SCP 1MB window maps to.
 * dram_1mb_window_address should be 1MB aligned with no trailing zeros
 * which means that the 1MB aligned base address should be right shifted by
 * 20 bits before passing to this function. Bit 0 of ADDR_TRANS register
 * is enable bit so the dram_1mb_window_address will be shifted left by
 * one bit before programming by this function.
 */
static void morello_system_enable_ap_memory_access(
    uint32_t dram_1mb_window_address)
{
    uint32_t addr_trans_reg = 0;

    addr_trans_reg = (dram_1mb_window_address << 1);
    addr_trans_reg |= ADDR_TRANS_EN;
    PIK_SCP->ADDR_TRANS = addr_trans_reg;
}

/*
 * Function to disable 1MB address translation window.
 */
static void morello_system_disable_ap_memory_access(void)
{
    PIK_SCP->ADDR_TRANS &= ~ADDR_TRANS_EN;
}

struct mod_morello_system_ap_memory_access_api
    morello_system_ap_memory_access_api = {
        .enable_ap_memory_access = morello_system_enable_ap_memory_access,
        .disable_ap_memory_access = morello_system_disable_ap_memory_access,
    };

/*
 * Function to copy into AP SRAM.
 */
static int morello_system_copy_to_ap_sram(
    uint32_t sram_address,
    const void *spi_address,
    uint32_t size)
{
    memcpy((void *)sram_address, spi_address, size);

    if (memcmp((void *)sram_address, spi_address, size) != 0) {
        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Copy failed at destination address: 0x%" PRIX32,
            sram_address);
        return FWK_E_DATA;
    }
    FWK_LOG_INFO(
        "[MORELLO SYSTEM] Copied binary to SRAM address: 0x%08" PRIX32,
        sram_address);
    return FWK_SUCCESS;
}

void cdbg_pwrupreq_handler(void)
{
    FWK_LOG_INFO("[MORELLO SYSTEM] Received debug power up request interrupt");
    FWK_LOG_INFO("[MORELLO SYSTEM] Power on Debug PIK");

    /* Clear interrupt */
    PIK_DEBUG->DEBUG_CTRL |= (0x1 << 1);
    fwk_interrupt_disable(CDBG_PWR_UP_REQ_IRQ);
}

void csys_pwrupreq_handler(void)
{
    FWK_LOG_INFO("[MORELLO SYSTEM] Received system power up request interrupt");

    /* Clear interrupt */
    PIK_DEBUG->DEBUG_CTRL |= (0x1 << 2);
    fwk_interrupt_disable(CSYS_PWR_UP_REQ_IRQ);
}

/*
 * Function to fill platform information structure.
 */
static int morello_system_fill_platform_info(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_platform_info_id);

    /* Force single chip mode with 8GB DDR DRAM */
    sds_platform_info.slave_count = 0;
    sds_platform_info.multichip_mode = 0;
    sds_platform_info.remote_ddr_size = 0;
    sds_platform_info.local_ddr_size = 8;

    FWK_LOG_INFO(
        "    Total DDR Size: %d GB",
        sds_platform_info.local_ddr_size + sds_platform_info.remote_ddr_size);

    return morello_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&sds_platform_info),
        sds_structure_desc->size);
}

static int morello_system_fill_bl33_info(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_bl33_info_id);

    sds_bl33_info.bl33_src_addr = BL33_SRC_BASE_ADDR;
    sds_bl33_info.bl33_dst_addr = BL33_DST_BASE_ADDR;
    sds_bl33_info.bl33_size = BL33_SIZE;
    return morello_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&sds_bl33_info),
        sds_structure_desc->size);
}

/*
 * Initialize primary core during system initialization
 */
static int morello_system_init_primary_core(void)
{
    int status;
    struct mod_pd_restricted_api *mod_pd_restricted_api = NULL;
    unsigned int core_idx;
    unsigned int cluster_idx;
    unsigned int cluster_count;

    FWK_LOG_INFO(
        "[MORELLO SYSTEM] Setting AP Reset Address to 0x%08" PRIX32,
        (AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET));

    cluster_count = morello_core_get_cluster_count();
    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        for (core_idx = 0;
             core_idx < morello_core_get_core_per_cluster_count(cluster_idx);
             core_idx++) {
            PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_LW =
                (AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET);
            PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_UP = 0;
        }
    }

    if (morello_get_chipid() == 0x0) {
        struct mod_fip_entry_data entry;
        status = morello_system_ctx.fip_api->get_entry(
            MOD_FIP_TOC_ENTRY_TFA_BL31, &entry);

        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO(
                "[MORELLO SYSTEM] Failed to locate AP TF_BL31, error: %d\n",
                status);
            return FWK_E_PANIC;
        }

        FWK_LOG_INFO("[MORELLO SYSTEM] Located AP TF_BL31:\n");
        FWK_LOG_INFO("[MORELLO SYSTEM]   address: %p\n", entry.base);
        FWK_LOG_INFO("[MORELLO SYSTEM]   size   : %u\n", entry.size);
        FWK_LOG_INFO(
            "[MORELLO SYSTEM]   flags  : 0x%08" PRIX32 "%08" PRIX32 "\n",
            (uint32_t)(entry.flags >> 32),
            (uint32_t)entry.flags);
        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Copying AP TF_BL31 to address 0x%" PRIX32 "...\n",
            AP_CORE_RESET_ADDR);

        status = morello_system_copy_to_ap_sram(
            AP_CORE_RESET_ADDR, entry.base, entry.size);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        /* Fill BL33 image information structure */
        FWK_LOG_INFO("[MORELLO SYSTEM] Filling BL33 information...");
        status = morello_system_fill_bl33_info();
        if (status != FWK_SUCCESS)
            return status;

        /* Fill Platform information structure */
        FWK_LOG_INFO("[MORELLO SYSTEM] Collecting Platform information...");
        status = morello_system_fill_platform_info();
        if (status != FWK_SUCCESS)
            return status;

        /* Enable non-secure CoreSight debug access */
        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Enabling CoreSight debug non-secure access");
        *(volatile uint32_t
              *)(AP_SCP_SRAM_OFFSET + NIC_400_SEC_0_CSAPBM_OFFSET) = 0xFFFFFFFF;

        mod_pd_restricted_api = morello_system_ctx.mod_pd_restricted_api;

        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Booting primary core at %lu MHz...",
            PIK_CLK_RATE_CLUS0_CPU / FWK_MHZ);

        status = mod_pd_restricted_api->set_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
            false,
            MOD_PD_COMPOSITE_STATE(
                MOD_PD_LEVEL_2,
                0,
                MOD_PD_STATE_ON,
                MOD_PD_STATE_ON,
                MOD_PD_STATE_ON));
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int messaging_stack_ready(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_feature_availability_id);

    /*
     * Write SDS Feature Availability to signal the completion of the messaging
     * stack
     */
    return morello_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&feature_flags),
        sds_structure_desc->size);
}

/*
 * Functions fulfilling the framework's module interface
 */

static int morello_system_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *unused2)
{
    int status;
    unsigned int idx;
    struct morello_system_isr *isr;

    for (idx = 0; idx < FWK_ARRAY_SIZE(isrs); idx++) {
        isr = &isrs[idx];
        status = fwk_interrupt_set_isr(isr->interrupt, isr->handler);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int morello_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_FIP),
        FWK_ID_API(FWK_MODULE_IDX_FIP, 0),
        &morello_system_ctx.fip_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &morello_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &morello_system_ctx.ppu_v1_isr_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(
        fwk_module_id_sds,
        FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
        &morello_system_ctx.sds_api);
}

static int morello_system_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_MORELLO_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER:
        *api = &morello_system_power_driver_api;
        break;
    case MOD_MORELLO_SYSTEM_API_IDX_AP_MEMORY_ACCESS:
        *api = &morello_system_ap_memory_access_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int morello_system_start(fwk_id_t id)
{
    int status;
    unsigned int i;
    uint32_t composite_state;

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_set_isr(CDBG_PWR_UP_REQ_IRQ, cdbg_pwrupreq_handler);
    if (status == FWK_SUCCESS) {
        fwk_interrupt_enable(CDBG_PWR_UP_REQ_IRQ);

        status =
            fwk_interrupt_set_isr(CSYS_PWR_UP_REQ_IRQ, csys_pwrupreq_handler);
        if (status == FWK_SUCCESS) {
            fwk_interrupt_enable(CSYS_PWR_UP_REQ_IRQ);

            PIK_CLUSTER(0)->CLKFORCE_SET = 0x00000004;
            PIK_CLUSTER(1)->CLKFORCE_SET = 0x00000004;

            /* Setup CoreSight counter */
            CS_CNTCONTROL->CS_CNTCR |= (1 << 0);
            CS_CNTCONTROL->CS_CNTCVLW = 0x00000000;
            CS_CNTCONTROL->CS_CNTCVUP = 0x0000FFFF;
        } else
            FWK_LOG_INFO(
                "[MORELLO SYSTEM] CSYS PWR UP REQ IRQ register failed");
    } else
        FWK_LOG_INFO("[MORELLO SYSTEM] CDBG PWR UP REQ IRQ register failed");

    FWK_LOG_INFO("[MORELLO SYSTEM] Requesting SYSTOP initialization...");

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
        if (status != FWK_SUCCESS)
            return status;
    }

    /*
     * Subscribe to the SDS initialized notification so we can correctly let the
     * PSCI agent know that the SCMI stack is initialized.
     */
    status = fwk_notification_subscribe(
        mod_sds_notification_id_initialized, fwk_module_id_sds, id);
    if (status != FWK_SUCCESS)
        return status;

    composite_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_OFF, MOD_PD_STATE_OFF);

    return morello_system_ctx.mod_pd_restricted_api->set_state_async(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0), false, composite_state);
}

static int morello_system_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params = NULL;
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;
    int status;

    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    params = (struct clock_notification_params *)event->params;

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        /*
         * Initialize primary core when the system is initialized for the
         * first time only.
         */
        if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
            status = morello_system_init_primary_core();
            if (status != FWK_SUCCESS)
                return status;

            /*
             * Unsubscribe to interconnect clock state change notification as
             * it has to be processed only once during system startup.
             */
            return fwk_notification_unsubscribe(
                event->id, event->source_id, event->target_id);
        }

        return FWK_SUCCESS;
    } else if (fwk_id_is_equal(
                   event->id, mod_scmi_notification_id_initialized)) {
        scmi_notification_count++;
    } else if (fwk_id_is_equal(
                   event->id, mod_sds_notification_id_initialized)) {
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

const struct fwk_module module_morello_system = {
    .name = "MORELLO_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_MORELLO_SYSTEM_API_COUNT,
    .init = morello_system_mod_init,
    .bind = morello_system_bind,
    .process_bind_request = morello_system_process_bind_request,
    .process_notification = morello_system_process_notification,
    .start = morello_system_start,
};
