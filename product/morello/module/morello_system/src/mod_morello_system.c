/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO System Support.
 */

#include "config_clock.h"
#include "morello_core.h"
#include "morello_mcp_scp.h"
#include "morello_pik_cpu.h"
#include "morello_pik_debug.h"
#include "morello_pik_scp.h"
#include "morello_scp_mmap.h"
#include "morello_scp_pik.h"
#include "morello_scp_scmi.h"
#include "morello_scp_system_mmap.h"
#include "morello_sds.h"

#include <mod_clock.h>
#include <mod_dmc_bing.h>
#if !defined(PLAT_FVP)
#    include <mod_morello_scp2pcc.h>
#    include <mod_morello_sensor.h>
#    include <mod_sensor.h>
#endif
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
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Platform information structure used by BL31
 */
struct FWK_PACKED morello_platform_info {
    /* Local DDR memory size in bytes */
    uint64_t local_ddr_size;
#if !defined(PLAT_FVP)
    /* Remote DDR memory size in bytes */
    uint64_t remote_ddr_size;
    /* Total number of remote chips  */
    uint8_t remote_chip_count;
    /* If multichip mode */
    bool multichip_mode;
    /* Platform SCC configuration */
    uint32_t scc_config;
    /* Silicon revision */
    uint32_t silicon_revision;
#endif
};

/*
 * Firmware version structure
 */
struct FWK_PACKED morello_firmware_version {
    uint32_t scp_fw_ver;
    uint32_t scp_fw_commit;
#if !defined(PLAT_FVP)
    uint32_t mcc_fw_ver;
    uint32_t pcc_fw_ver;
#endif
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

/* SDS Firmware version information */
static struct morello_firmware_version sds_firmware_version;
static fwk_id_t sds_firmware_version_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, SDS_ELEMENT_IDX_FIRMWARE_VERSION);

/* Module context */
struct morello_system_ctx {
    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* Pointer to DMC Bing memory information API */
    const struct mod_dmc_bing_mem_info_api *dmc_bing_api;

    /* Pointer to SDS */
    const struct mod_sds_api *sds_api;

#if !defined(PLAT_FVP)
    /* Pointer to SCP to PCC communication API */
    const struct mod_morello_scp2pcc_api *scp2pcc_api;
    /* Pointer to Sensor API */
    struct mod_sensor_driver_api *morello_sensor_api;
#endif
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
#if !defined(PLAT_FVP)
    int status;

    switch (system_shutdown) {
    case MOD_PD_SYSTEM_SHUTDOWN:
        FWK_LOG_INFO("[MORELLO SYSTEM] Request PCC for system shutdown");
        status = morello_system_ctx.scp2pcc_api->send(
            MOD_SCP2PCC_SEND_SHUTDOWN, NULL, 0, NULL, NULL);
        break;

    case MOD_PD_SYSTEM_COLD_RESET:
        FWK_LOG_INFO("[MORELLO SYSTEM] Request PCC for system reboot");
        status = morello_system_ctx.scp2pcc_api->send(
            MOD_SCP2PCC_SEND_REBOOT, NULL, 0, NULL, NULL);
        break;
    default:
        FWK_LOG_INFO("[MORELLO SYSTEM] Unknown shutdown command!");
        status = FWK_E_PARAM;
        break;
    }

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[MORELLO SYSTEM] Shutdown/Reboot request to PCC failed");
        return status;
    }
#endif

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
 * Function to read commit id from Morello build string
 */
static int morello_system_read_commit_id(uint32_t *scp_commit)
{
    unsigned int build_string_idx;
    unsigned int commit_hex_count;
    char build_string[] = BUILD_VERSION_DESCRIBE_STRING;
    char *separator_ptr;

    /* Check string is null-terminated */
    if (build_string[sizeof(build_string) - 1] != '\0') {
        return FWK_E_PARAM;
    }

    /* Version characters before separator can be ignored */
    separator_ptr = memchr(build_string, '_', sizeof(build_string));
    if (separator_ptr == NULL) {
        return FWK_E_PARAM;
    }

    build_string_idx = separator_ptr - build_string;

    /* Add each hex into the commit ID */
    for (build_string_idx++, commit_hex_count = 0;
         build_string[build_string_idx] != '\0' &&
         commit_hex_count < MORELLO_SDS_FIRMWARE_COMMIT_ID_LEN;
         build_string_idx++, commit_hex_count++) {
        if ((build_string[build_string_idx] >= '0') &&
            (build_string[build_string_idx] <= '9')) {
            *scp_commit =
                ((*scp_commit << 4) | (0x0F & build_string[build_string_idx]));
        } else if (
            (build_string[build_string_idx] >= 'a') &&
            (build_string[build_string_idx] <= 'f')) {
            *scp_commit =
                ((*scp_commit << 4) |
                 (0x0F & (build_string[build_string_idx] + 9)));
        } else {
            return FWK_E_PARAM;
        }
    }

    return FWK_SUCCESS;
}

/*
 * Function to fill platform information structure.
 */
static int morello_system_fill_platform_info(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_platform_info_id);
    uint64_t size = 0;
    int status;
#if !defined(PLAT_FVP)
    uint16_t length;
    uint8_t silicon_rev[MORELLO_SCP2PCC_SILICON_REVISION_LEN];
#endif

    status = morello_system_ctx.dmc_bing_api->get_mem_size(&size);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("Error calculating local DDR memory size!");
        return status;
    }
    sds_platform_info.local_ddr_size = size;

#if !defined(PLAT_FVP)
    /* Force single chip mode */
    sds_platform_info.remote_chip_count = 0;
    sds_platform_info.multichip_mode = 0;
    sds_platform_info.remote_ddr_size = 0;

    size = sds_platform_info.local_ddr_size + sds_platform_info.remote_ddr_size;

    /* Account for size reserved for tag bits storage in dmc-bing client mode */
    if (SCC->BOOT_GPR1 & 0x1) {
        size -= (size / UINT64_C(128));
    }

    (void)size;
    FWK_LOG_INFO(
        "[MORELLO SYSTEM] Total usable DDR size: %" PRIu32 "MB",
        (uint32_t)(size / FWK_MIB));

    sds_platform_info.scc_config = SCC->BOOT_GPR1;

    /* Get the Silicon revision */
    status = morello_system_ctx.scp2pcc_api->send(
        MOD_SCP2PCC_GET_SILICON_REVISION, NULL, 0, &silicon_rev, &length);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[MORELLO SYSTEM] Error fetching the silicon revision");
        return status;
    }

    if (length != MORELLO_SCP2PCC_SILICON_REVISION_LEN) {
        FWK_LOG_ERR(
            "[MORELLO SYSTEM] Unexpected silicon revision length %" PRIu16,
            length);
        return FWK_E_DATA;
    }

    sds_platform_info.silicon_revision =
        (silicon_rev[MORELLO_SCP2PCC_SILICON_REVISION_P_INDEX] +
         (silicon_rev[MORELLO_SCP2PCC_SILICON_REVISION_R_INDEX]
          << MORELLO_SILICON_REVISION_R_POS));
#endif

    return morello_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&sds_platform_info),
        sds_structure_desc->size);
}

/*
 * Function to fill firmware version information structure.
 */
static int morello_system_fill_firmware_versions(void)
{
    int status;
    uint32_t scp_commit_id = 0;
#if !defined(PLAT_FVP)
    uint16_t length;
    uint8_t pcc_version[MORELLO_SCP2PCC_PCC_FW_VERSION_LEN];
    uint8_t mcc_version[MORELLO_SCP2PCC_MCC_FW_VERSION_LEN];
#endif

    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_firmware_version_id);

    status = morello_system_read_commit_id(&scp_commit_id);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[MORELLO SYSTEM] Unable to parse the build string, set default");
        sds_firmware_version.scp_fw_ver = 0;
        sds_firmware_version.scp_fw_commit = 0;
    } else {
        sds_firmware_version.scp_fw_ver = FWK_BUILD_VERSION;
        sds_firmware_version.scp_fw_commit = scp_commit_id;
    }

#if !defined(PLAT_FVP)
    /* Get the PCC firmware version */
    FWK_LOG_INFO("[MORELLO SYSTEM] Fetching PCC firmware version");
    status = morello_system_ctx.scp2pcc_api->send(
        MOD_SCP2PCC_GET_PCC_FW_VERSION, NULL, 0, pcc_version, &length);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[MORELLO SYSTEM] Error fetching the PCC version");
        return status;
    }

    if (length != MORELLO_SCP2PCC_PCC_FW_VERSION_LEN) {
        FWK_LOG_ERR(
            "[MORELLO SYSTEM] Unexpected PCC version length %" PRIu16, length);
        return FWK_E_DATA;
    }

    sds_firmware_version.pcc_fw_ver =
        ((pcc_version[MORELLO_SCP2PCC_PCC_FW_VERSION_UPPER_INDEX]
          << MORELLO_PCC_FIRMWARE_VERSION_UPPER_POS) |
         (pcc_version[MORELLO_SCP2PCC_PCC_FW_VERSION_MID_INDEX]
          << MORELLO_PCC_FIRMWARE_VERSION_MID_POS) |
         (pcc_version[MORELLO_SCP2PCC_PCC_FW_VERSION_LOWER_INDEX]
          << MORELLO_PCC_FIRMWARE_VERSION_LOWER_POS));

    /* Get the MCC firmware version */
    FWK_LOG_INFO("[MORELLO SYSTEM] Fetching MCC firmware version");
    status = morello_system_ctx.scp2pcc_api->send(
        MOD_SCP2PCC_GET_MCC_FW_VERSION, NULL, 0, mcc_version, &length);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[MORELLO SYSTEM] Error fetching the MCC version");
        return status;
    }

    if (length != MORELLO_SCP2PCC_MCC_FW_VERSION_LEN) {
        FWK_LOG_ERR(
            "[MORELLO SYSTEM] Unexpected MCC version length %" PRIu16, length);
        return FWK_E_DATA;
    }

    sds_firmware_version.mcc_fw_ver =
        ((mcc_version[MORELLO_SCP2PCC_MCC_FW_VERSION_UPPER_INDEX]
          << MORELLO_MCC_FIRMWARE_VERSION_UPPER_POS) |
         (mcc_version[MORELLO_SCP2PCC_MCC_FW_VERSION_MID_INDEX]
          << MORELLO_MCC_FIRMWARE_VERSION_MID_POS) |
         (mcc_version[MORELLO_SCP2PCC_MCC_FW_VERSION_LOWER_INDEX]
          << MORELLO_MCC_FIRMWARE_VERSION_LOWER_POS));
#endif

    return morello_system_ctx.sds_api->struct_write(
        sds_structure_desc->id,
        0,
        (void *)(&sds_firmware_version),
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

#if !defined(PLAT_FVP)
    uint64_t temperature[MOD_MORELLO_SYSTEM_SENSOR_COUNT] = { 0, 0, 0 };
#endif

    FWK_LOG_INFO(
        "[MORELLO SYSTEM] Setting AP Reset Address to 0x%08" PRIX32
        "%08" PRIX32,
        SCC->BOOT_GPR3,
        SCC->BOOT_GPR2);

    cluster_count = morello_core_get_cluster_count();
    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        for (core_idx = 0;
             core_idx < morello_core_get_core_per_cluster_count(cluster_idx);
             core_idx++) {
            PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_LW =
                SCC->BOOT_GPR2;
            PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_UP =
                SCC->BOOT_GPR3;
        }
    }

    if (morello_get_chipid() == 0x0) {
        /* Fill Platform information structure */
        FWK_LOG_INFO("[MORELLO SYSTEM] Collecting Platform information...");
        status = morello_system_fill_platform_info();
        if (status != FWK_SUCCESS)
            return status;

        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Collecting Firmware version information...");
        status = morello_system_fill_firmware_versions();
        if (status != FWK_SUCCESS)
            return status;

        /*
         * At the moment, DPU ACLK is not hooked to SCMI Clock protocol due to
         * an issue with clock dividier configuration.
         * As such, hard-coding the dividers here until SCMI Clock comms
         * is fixed to work with DPU ALCK.
         */
        PIK_DPU->ACLKDP_DIV1 = 0;
        PIK_DPU->ACLKDP_DIV2 = 0;

        /*
         * To achieve full speed operation (150MHz) the Trace interface outputs
         * on the SoC require their drive strength to be increased from 8mA to
         * 12mA and the slew rate to be changed from SLOW to FAST in the
         * respective SCC registers. This change applies to both the data and
         * clk signals (total = 34).
         */
        SCC->TRACE_PAD_CTRL0 = UINT32_C(0x3030303);
        SCC->TRACE_PAD_CTRL1 = UINT32_C(0x303);

        /*
         * To achieve full-speed operation (150MHz) on the display output on
         * the SoC, the display pads require their slew rate to be changed from
         * FAST to SLOW.
         * This change is only needed for the pixel clock pad.
         */
        SCC->DISPLAY_PAD_CTRL0 = UINT32_C(0x01010111);

        /* Enable non-secure CoreSight debug access */
        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Enabling CoreSight debug non-secure access");
        *(volatile uint32_t
              *)(AP_SCP_SRAM_OFFSET + NIC_400_SEC_0_CSAPBM_OFFSET) = 0xFFFFFFFF;

        mod_pd_restricted_api = morello_system_ctx.mod_pd_restricted_api;

#if !defined(PLAT_FVP)
        for (int sensor_idx = 0; sensor_idx < MOD_MORELLO_SYSTEM_SENSOR_COUNT;
             sensor_idx++) {
            /* Get the Temperature Values */
            status = morello_system_ctx.morello_sensor_api->get_value(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_MORELLO_SENSOR, sensor_idx),
                &temperature[sensor_idx]);
            if (status != FWK_SUCCESS) {
                FWK_LOG_INFO(
                    "[MORELLO SYSTEM] Error getting the temperature"
                    " value!");
                return status;
            }
        }

        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Temp of CLUS0 = %uC, CLUS1 = %uC, SYS = %uC",
            (unsigned int)temperature[MOD_MORELLO_SYSTEM_CLUSTER0_SENSOR],
            (unsigned int)temperature[MOD_MORELLO_SYSTEM_CLUSTER1_SENSOR],
            (unsigned int)temperature[MOD_MORELLO_SYSTEM_SENSOR]);
#endif

        FWK_LOG_INFO(
            "[MORELLO SYSTEM] Booting primary core at %lu MHz...",
            PIK_CLK_RATE_CLUS0_CPU / FWK_MHZ);

        status = mod_pd_restricted_api->set_state(
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

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_DMC_BING),
        FWK_ID_API(FWK_MODULE_IDX_DMC_BING, MOD_DDR_API_IDX_MEM_INFO),
        &morello_system_ctx.dmc_bing_api);
    if (status != FWK_SUCCESS)
        return status;

#if !defined(PLAT_FVP)
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_SCP2PCC),
        FWK_ID_API(FWK_MODULE_IDX_MORELLO_SCP2PCC, 0),
        &morello_system_ctx.scp2pcc_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_SENSOR),
        FWK_ID_API(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        &morello_system_ctx.morello_sensor_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

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
        } else {
            FWK_LOG_INFO(
                "[MORELLO SYSTEM] CSYS PWR UP REQ IRQ register failed");
        }
    } else {
        FWK_LOG_INFO("[MORELLO SYSTEM] CDBG PWR UP REQ IRQ register failed");
    }

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

    return morello_system_ctx.mod_pd_restricted_api->set_state(
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
            /*
             * Write a handshake pattern to MCP2SCP Secure MHU RAM to let MCP
             * know that it can continue the boot.
             */
            *(FWK_W uint32_t *)SCP_MCP_SHARED_SECURE_RAM =
                MORELLO_SCP_MCP_HANDSHAKE_PATTERN;
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
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_MORELLO_SYSTEM_API_COUNT,
    .init = morello_system_mod_init,
    .bind = morello_system_bind,
    .process_bind_request = morello_system_process_bind_request,
    .process_notification = morello_system_process_notification,
    .start = morello_system_start,
};
