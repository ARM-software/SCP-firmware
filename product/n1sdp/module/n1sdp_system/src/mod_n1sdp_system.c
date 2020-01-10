/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP System Support.
 */

#include "config_clock.h"
#include "n1sdp_core.h"
#include "n1sdp_pik_cpu.h"
#include "n1sdp_pik_debug.h"
#include "n1sdp_pik_scp.h"
#include "n1sdp_scp_irq.h"
#include "n1sdp_scp_mmap.h"
#include "n1sdp_scp_pik.h"
#include "n1sdp_scp_scmi.h"
#include "n1sdp_sds.h"

#include <internal/n1sdp_scp2pcc.h>

#include <mod_clock.h>
#include <mod_n1sdp_c2c_i2c.h>
#include <mod_n1sdp_dmc620.h>
#include <mod_n1sdp_flash.h>
#include <mod_n1sdp_scp2pcc.h>
#include <mod_n1sdp_system.h>
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
struct n1sdp_platform_info {
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
struct n1sdp_bl33_info {
    /* Source address of BL33 image */
    uint32_t bl33_src_addr;
    /* Load address of BL33 image */
    uint32_t bl33_dst_addr;
    /* BL33 image size */
    uint32_t bl33_size;
};

/* MultiChip information */
struct n1sdp_multichip_info {
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
    FWK_R  uint32_t CS_CNTSR;
    FWK_RW uint32_t CS_CNTCVLW;
    FWK_RW uint32_t CS_CNTCVUP;
};

#define CS_CNTCONTROL ((struct cs_cnt_ctrl_reg *)SCP_CS_CNTCONTROL_BASE)

/* SCMI Services used by software on the AP cores */
static unsigned int scmi_notification_table[] = {
    SCP_N1SDP_SCMI_SERVICE_IDX_PSCI,
    SCP_N1SDP_SCMI_SERVICE_IDX_OSPM,
};

/* SDS Feature flags */
static const uint32_t feature_flags = (N1SDP_SDS_FEATURE_FIRMWARE_MASK |
                                       N1SDP_SDS_FEATURE_DMC_MASK |
                                       N1SDP_SDS_FEATURE_MESSAGING_MASK);
static fwk_id_t sds_feature_availability_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS,
                            SDS_ELEMENT_IDX_FEATURE_AVAILABILITY);

/* SDS DDR memory information */
static struct n1sdp_platform_info sds_platform_info;
static fwk_id_t sds_platform_info_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, SDS_ELEMENT_IDX_PLATFORM_INFO);

/* SDS BL33 image information */
static struct n1sdp_bl33_info sds_bl33_info;
static fwk_id_t sds_bl33_info_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, SDS_ELEMENT_IDX_BL33_INFO);

/* Module context */
struct n1sdp_system_ctx {
    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* Pointer to N1SDP Flash APIs */
    const struct mod_n1sdp_flash_api *flash_api;

    /* Pointer to DMC620 memory information API */
    const struct mod_dmc620_mem_info_api *dmc620_api;

    /* Pointer to SDS */
    const struct mod_sds_api *sds_api;

    /* Pointer to N1SDP C2C slave information API */
    const struct n1sdp_c2c_slave_info_api *c2c_api;

    /* Pointer to N1SDP C2C PD API */
    const struct n1sdp_c2c_pd_api *c2c_pd_api;

    /* Pointer to SCP to PCC communication API */
    const struct mod_n1sdp_scp2pcc_api *scp2pcc_api;
};

struct n1sdp_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct n1sdp_system_ctx n1sdp_system_ctx;
const struct fwk_module_config config_n1sdp_system = { 0 };

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

        if (core_idx >= n1sdp_core_get_core_count())
            continue;

        n1sdp_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, core_idx));
    }
}

static void ppu_clusters_isr(void)
{
    uint32_t status = PIK_SCP->CLUS_PPU_INT_STATUS;
    unsigned int cluster_idx;

    while (status != 0) {
        cluster_idx = __builtin_ctz(status);

        n1sdp_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1,
            n1sdp_core_get_core_count() + cluster_idx));

        status &= ~(1 << cluster_idx);
    }
}

/*
 *  PPU Interrupt Service Routine table
 */

static struct n1sdp_system_isr isrs[] = {
    [0] = { .interrupt = PPU_CORES0_IRQ,
            .handler = ppu_cores_isr },
    [1] = { .interrupt = PPU_CLUSTERS_IRQ,
            .handler = ppu_clusters_isr },
};

/*
 * System power module's driver API
 */

static int n1sdp_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
    /* Check if we are running in multichip configuration */
    if (n1sdp_system_ctx.c2c_api->is_slave_alive()) {
        n1sdp_system_ctx.c2c_pd_api->shutdown_reboot(
            N1SDP_C2C_CMD_SHUTDOWN_OR_REBOOT, system_shutdown);
    }

    switch (system_shutdown) {
    case MOD_PD_SYSTEM_SHUTDOWN:
        FWK_LOG_INFO("[N1SDP SYSTEM] Request PCC for system shutdown");
        n1sdp_system_ctx.scp2pcc_api->send(NULL, 0, SCP2PCC_TYPE_SHUTDOWN);
        break;

    case MOD_PD_SYSTEM_COLD_RESET:
        FWK_LOG_INFO("[N1SDP SYSTEM] Request PCC for system reboot");
        n1sdp_system_ctx.scp2pcc_api->send(NULL, 0, SCP2PCC_TYPE_REBOOT);
        break;

    default:
        FWK_LOG_INFO("[N1SDP SYSTEM] Unknown shutdown command!");
        break;
    }

    NVIC_SystemReset();
    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    n1sdp_system_power_driver_api = {
    .system_shutdown = n1sdp_system_shutdown,
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
static void n1sdp_system_enable_ap_memory_access(
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
static void n1sdp_system_disable_ap_memory_access(void)
{
    PIK_SCP->ADDR_TRANS &= ~ADDR_TRANS_EN;
}

struct mod_n1sdp_system_ap_memory_access_api
    n1sdp_system_ap_memory_access_api = {
        .enable_ap_memory_access = n1sdp_system_enable_ap_memory_access,
        .disable_ap_memory_access = n1sdp_system_disable_ap_memory_access,
};

/*
 * Function to copy into AP SRAM.
 */
static int n1sdp_system_copy_to_ap_sram(uint64_t sram_address,
                                        uint32_t spi_address,
                                        uint32_t size)
{
    uint32_t target_addr = (uint32_t)sram_address;

    memcpy((void *)target_addr, (void *)spi_address, size);

    if (memcmp((void *)target_addr, (void *)spi_address, size) != 0) {
        FWK_LOG_INFO(
            "[N1SDP SYSTEM] Copy failed at destination address: 0x%" PRIX32,
            target_addr);
        return FWK_E_DATA;
    }
    FWK_LOG_INFO(
        "[N1SDP SYSTEM] Copied binary to SRAM address: 0x%" PRIX64,
        sram_address);
    return FWK_SUCCESS;
}

void cdbg_pwrupreq_handler(void)
{
    FWK_LOG_INFO("[N1SDP SYSTEM] Received debug power up request interrupt");

    FWK_LOG_INFO("[N1SDP SYSTEM] Power on Debug PIK");

    /* Clear interrupt */
    PIK_DEBUG->DEBUG_CTRL |= (0x1 << 1);
    fwk_interrupt_disable(CDBG_PWR_UP_REQ_IRQ);
}

void csys_pwrupreq_handler(void)
{
    FWK_LOG_INFO("[N1SDP SYSTEM] Received system power up request interrupt");

    /* Clear interrupt */
    PIK_DEBUG->DEBUG_CTRL |= (0x1 << 2);
    fwk_interrupt_disable(CSYS_PWR_UP_REQ_IRQ);
}

/*
 * Function to fill platform information structure.
 */
static int n1sdp_system_fill_platform_info(void)
{
    int status;
    uint32_t ddr_size_gb;

    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_platform_info_id);

    sds_platform_info.slave_count = 0;
    sds_platform_info.remote_ddr_size = 0;

    status = n1sdp_system_ctx.dmc620_api->get_mem_size_gb(&ddr_size_gb);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("Error calculating local DDR memory size!");
        return status;
    }

    sds_platform_info.local_ddr_size = ddr_size_gb;

    /* Get remote chip's information */
    sds_platform_info.multichip_mode = n1sdp_system_ctx.c2c_api->
                                          is_slave_alive();

    if (sds_platform_info.multichip_mode) {
        sds_platform_info.slave_count = 1;
        status = n1sdp_system_ctx.c2c_api->get_ddr_size_gb
                                           (&sds_platform_info.remote_ddr_size);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("Error calculating Remote DDR memory size!");
            return status;
        }
    }

    FWK_LOG_INFO(
        "    Total DDR Size: %d GB",
        sds_platform_info.local_ddr_size + sds_platform_info.remote_ddr_size);

    return n1sdp_system_ctx.sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&sds_platform_info), sds_structure_desc->size);
}

static int n1sdp_system_fill_bl33_info(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_bl33_info_id);

    sds_bl33_info.bl33_src_addr = BL33_SRC_BASE_ADDR;
    sds_bl33_info.bl33_dst_addr = BL33_DST_BASE_ADDR;
    sds_bl33_info.bl33_size = BL33_SIZE;
    return n1sdp_system_ctx.sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&sds_bl33_info), sds_structure_desc->size);
}

/*
 * Initialize primary core during system initialization
 */
static int n1sdp_system_init_primary_core(void)
{
    int status;
    struct mod_pd_restricted_api *mod_pd_restricted_api = NULL;
    struct mod_n1sdp_fip_descriptor *fip_desc_table = NULL;
    unsigned int fip_count;
    unsigned int i;
    unsigned int core_idx;
    unsigned int cluster_idx;
    unsigned int cluster_count;
    int fip_index_bl31 = -1;

    FWK_LOG_INFO(
        "[N1SDP SYSTEM] Setting AP Reset Address to 0x%" PRIX64,
        AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET);

    cluster_count = n1sdp_core_get_cluster_count();
    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        for (core_idx = 0;
            core_idx < n1sdp_core_get_core_per_cluster_count(cluster_idx);
            core_idx++) {
            PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_LW
                = (uint32_t)(AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET);
            PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_UP
                = (uint32_t)
                ((AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET) >> 32);
        }
    }

    if (n1sdp_get_chipid() == 0x0) {
        FWK_LOG_INFO(
            "[N1SDP SYSTEM] Looking for AP firmware in flash memory...");

        status = n1sdp_system_ctx.flash_api->get_n1sdp_fip_descriptor_count(
            FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SYSTEM), &fip_count);
        if (status != FWK_SUCCESS)
            return status;

        status = n1sdp_system_ctx.flash_api->get_n1sdp_fip_descriptor_table(
            FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SYSTEM), &fip_desc_table);
        if (status != FWK_SUCCESS)
            return status;

        for (i = 0; i < fip_count; i++) {
            if (fip_desc_table[i].type == MOD_N1SDP_FIP_TYPE_TF_BL31) {
                FWK_LOG_INFO(
                    "[N1SDP SYSTEM] Found BL31 at address: 0x%08x,"
                    " size: %u, flags: 0x%x",
                    fip_desc_table[i].address,
                    fip_desc_table[i].size,
                    fip_desc_table[i].flags);
                fip_index_bl31 = i;
                break;
            }
        }

        if (fip_index_bl31 < 0) {
            FWK_LOG_INFO(
                "[N1SDP SYSTEM] Error! "
                "FIP does not have BL31 binary");
            return FWK_E_PANIC;
        }

        FWK_LOG_INFO(
            "[N1SDP SYSTEM] Copying AP BL31 to address 0x%" PRIX64 "...",
            AP_CORE_RESET_ADDR);

        status = n1sdp_system_copy_to_ap_sram(AP_CORE_RESET_ADDR,
                     fip_desc_table[fip_index_bl31].address,
                     fip_desc_table[fip_index_bl31].size);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        /* Fill BL33 image information structure */
        FWK_LOG_INFO("[N1SDP SYSTEM] Filling BL33 information...");
        status = n1sdp_system_fill_bl33_info();
        if (status != FWK_SUCCESS)
            return status;

        /* Fill Platform information structure */
        FWK_LOG_INFO("[N1SDP SYSTEM] Collecting Platform information...");
        status = n1sdp_system_fill_platform_info();
        if (status != FWK_SUCCESS)
            return status;

        /* Enable non-secure CoreSight debug access */
        FWK_LOG_INFO(
            "[N1SDP SYSTEM] Enabling CoreSight debug non-secure access");
        *(volatile uint32_t *)(AP_SCP_SRAM_OFFSET +
                               NIC_400_SEC_0_CSAPBM_OFFSET) = 0xFFFFFFFF;

        mod_pd_restricted_api = n1sdp_system_ctx.mod_pd_restricted_api;

        FWK_LOG_INFO(
            "[N1SDP SYSTEM] Booting primary core at %lu MHz...",
            PIK_CLK_RATE_CLUS0_CPU / FWK_MHZ);

        status = mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
            false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                MOD_PD_STATE_ON, MOD_PD_STATE_ON));
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
    return n1sdp_system_ctx.sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&feature_flags), sds_structure_desc->size);
}

/*
 * Functions fulfilling the framework's module interface
 */

static int n1sdp_system_mod_init(fwk_id_t module_id, unsigned int unused,
    const void *unused2)
{
    int status;
    unsigned int idx;
    struct n1sdp_system_isr *isr;

    for (idx = 0; idx < FWK_ARRAY_SIZE(isrs); idx++) {
        isr = &isrs[idx];
        status = fwk_interrupt_set_isr(isr->interrupt, isr->handler);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int n1sdp_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_FLASH),
         FWK_ID_API(FWK_MODULE_IDX_N1SDP_FLASH, 0),
         &n1sdp_system_ctx.flash_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_DMC620),
         FWK_ID_API(FWK_MODULE_IDX_N1SDP_DMC620, MOD_DMC620_API_IDX_MEM_INFO),
         &n1sdp_system_ctx.dmc620_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &n1sdp_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &n1sdp_system_ctx.ppu_v1_isr_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_C2C),
        FWK_ID_API(FWK_MODULE_IDX_N1SDP_C2C, N1SDP_C2C_API_IDX_SLAVE_INFO),
        &n1sdp_system_ctx.c2c_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_C2C),
        FWK_ID_API(FWK_MODULE_IDX_N1SDP_C2C, N1SDP_C2C_API_IDX_PD),
        &n1sdp_system_ctx.c2c_pd_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SCP2PCC),
        FWK_ID_API(FWK_MODULE_IDX_N1SDP_SCP2PCC, 0),
        &n1sdp_system_ctx.scp2pcc_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(fwk_module_id_sds,
        FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
        &n1sdp_system_ctx.sds_api);
}

static int n1sdp_system_process_bind_request(fwk_id_t requester_id,
    fwk_id_t pd_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_N1SDP_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER:
        *api = &n1sdp_system_power_driver_api;
        break;
    case MOD_N1SDP_SYSTEM_API_IDX_AP_MEMORY_ACCESS:
        *api = &n1sdp_system_ap_memory_access_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int n1sdp_system_start(fwk_id_t id)
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

        status = fwk_interrupt_set_isr(CSYS_PWR_UP_REQ_IRQ,
            csys_pwrupreq_handler);
        if (status == FWK_SUCCESS) {
            fwk_interrupt_enable(CSYS_PWR_UP_REQ_IRQ);

            PIK_CLUSTER(0)->CLKFORCE_SET = 0x00000004;
            PIK_CLUSTER(1)->CLKFORCE_SET = 0x00000004;

            /* Setup CoreSight counter */
            CS_CNTCONTROL->CS_CNTCR |= (1 << 0);
            CS_CNTCONTROL->CS_CNTCVLW = 0x00000000;
            CS_CNTCONTROL->CS_CNTCVUP = 0x0000FFFF;
        } else
            FWK_LOG_ERR("[N1SDP SYSTEM] CSYS PWR UP REQ IRQ register failed");
    } else
        FWK_LOG_ERR("[N1SDP SYSTEM] CDBG PWR UP REQ IRQ register failed");

    FWK_LOG_INFO("[N1SDP SYSTEM] Requesting SYSTOP initialization...");

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

    if (n1sdp_is_multichip_enabled() && (n1sdp_get_chipid() == 0x0)) {
        composite_state = MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_3,
                                                 MOD_PD_STATE_ON,
                                                 MOD_PD_STATE_ON,
                                                 MOD_PD_STATE_OFF,
                                                 MOD_PD_STATE_OFF);
    } else {
        composite_state = MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2,
                                                 0,
                                                 MOD_PD_STATE_ON,
                                                 MOD_PD_STATE_OFF,
                                                 MOD_PD_STATE_OFF);
    }

    return n1sdp_system_ctx.mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0), false,
            composite_state);
}

static int n1sdp_system_process_notification(const struct fwk_event *event,
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
            status = n1sdp_system_init_primary_core();
            if (status != FWK_SUCCESS)
                return status;

            /*
             * Unsubscribe to interconnect clock state change notification as
             * it has to be processed only once during system startup.
             */
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

const struct fwk_module module_n1sdp_system = {
    .name = "N1SDP_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_N1SDP_SYSTEM_API_COUNT,
    .init = n1sdp_system_mod_init,
    .bind = n1sdp_system_bind,
    .process_bind_request = n1sdp_system_process_bind_request,
    .process_notification = n1sdp_system_process_notification,
    .start = n1sdp_system_start,
};
