/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Use of "manager" may be out of sync with older versions of TRM */

#include "bootctl_reg.h"
#include "low_level_access.h"
#include "pik_system.h"
#include "synquacer_common.h"
#include "synquacer_config.h"
#include "synquacer_mmap.h"
#include "fwu_mdata.h"

#include <boot_ctl.h>
#include <sysdef_option.h>

#include <internal/crg11.h>
#include <internal/gpio.h>
#include <internal/nic400.h>
#include <internal/thermal_sensor.h>

#include <mod_f_i2c.h>
#include <mod_hsspi.h>
#include <mod_nor.h>
#include <mod_power_domain.h>
#include <mod_synquacer_system.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SEC_OVERRIDE_CONVERT_MASK UINT32_C(0x3)

#define SEC_OVERRIDE_CONVERT_TO_SEC UINT8_C(0)
#define SEC_OVERRIDE_CONVERT_TO_NONSEC UINT8_C(1)
#define SEC_OVERRIDE_CONVERT_TO_NO_CHANGE UINT8_C(2)

#define SEC_OVERRIDE_REG_PCIEIF0_MMU UINT8_C(0)
#define SEC_OVERRIDE_REG_PCIEIF1_MMU UINT8_C(2)
#define SEC_OVERRIDE_REG_EMMC_XDMAC_NETSEC_MMU UINT8_C(4)
#define SEC_OVERRIDE_REG_XDMAC_NIC UINT8_C(16)
#define SEC_OVERRIDE_REG_EMMC_NIC UINT8_C(18)
#define SEC_OVERRIDE_REG_NETSEC_NIC UINT8_C(20)

eeprom_config_t eeprom_config;

static fwk_id_t qspi_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_HSSPI, 0);
static fwk_id_t nor_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_NOR, 0);

void power_domain_coldboot(void);
int fw_ddr_spd_param_check(void);
void bus_sysoc_init(void);
void fw_fip_load_bl2(uint32_t boot_index);
void fw_fip_load_arm_tf(void);
void smmu_wrapper_initialize(void);
void pcie_wrapper_configure(void);

static void fw_prepare_debug_pik(void)
{
    /* DEBUG ack */
    /*
     * bit2 : CSYSPWRUPACK
     * bit1 : CDBGPWRUPACK
     * bit0 : CDBGPWRUPACK
     */
    writel(PIK_DEBUG_BASE, 0x00000007);
}

void fw_ap_data_clk_preparation(void)
{
    struct ap_data {
        /* Use 8 bit temporarily until SCP and TF team get aligned*/
        volatile uint64_t a72_cluster_mask : 8;
        volatile uint32_t primary_cpu : 6;
        volatile uint32_t unused : 18;
    } *ap_data;

    /* Initialise AP context area */
    memset((void *)AP_CONTEXT_BASE, 0, AP_CONTEXT_SIZE);

    /*
     * Pass data to the AP using the MHU secure payload area for cluster 1.
     * Avoid using cluster 0's payload area as this will be overwritten by the
     * SCP Ready message before the AP is powered-up and has a chance to copy
     * the data.
     */
    ap_data = ((struct ap_data *)MHU_PAYLOAD_S_CLUSTER_BASE(1));
    ap_data->a72_cluster_mask = 0;
    ap_data->primary_cpu = 0;
    ap_data->unused = 0;
    __DMB();

    fw_prepare_debug_pik();
}

eeprom_config_t *fw_get_config_table(void)
{
    return &eeprom_config;
}

void fw_set_sec_override(uint8_t value, uint8_t change_bit_offset)
{
    uint32_t temp, intsts;

    if (value >= 0x3U)
        return;

    if (change_bit_offset > 20U)
        return;

    DI(intsts);

    temp = readl(
        CONFIG_SOC_REG_ADDR_SYS_OVER_REG_TOP +
        CONFIG_SOC_SYS_OVER_OFFSET_SEC_OVERRIDE);

    /* SEC_OVERRIDE_SCBM_MMU Set Convert to the Secure Transaction. */
    temp &= (~(SEC_OVERRIDE_CONVERT_MASK << change_bit_offset));
    temp |= (value << change_bit_offset);
    writel(
        CONFIG_SOC_REG_ADDR_SYS_OVER_REG_TOP +
            CONFIG_SOC_SYS_OVER_OFFSET_SEC_OVERRIDE,
        temp);

    EI(intsts);
}

static void scb_am_block_init(void)
{
    uint32_t value;

    /*
     * set emmc/netsec1/xdmac to non-secure so that CA53 can use
     * as a non-secure manager
     */
    fw_set_sec_override(
        SEC_OVERRIDE_CONVERT_TO_NONSEC, SEC_OVERRIDE_REG_EMMC_XDMAC_NETSEC_MMU);
    fw_set_sec_override(
        SEC_OVERRIDE_CONVERT_TO_NONSEC, SEC_OVERRIDE_REG_XDMAC_NIC);
    fw_set_sec_override(
        SEC_OVERRIDE_CONVERT_TO_NONSEC, SEC_OVERRIDE_REG_EMMC_NIC);
    fw_set_sec_override(
        SEC_OVERRIDE_CONVERT_TO_NONSEC, SEC_OVERRIDE_REG_NETSEC_NIC);

#if defined(SET_PCIE_NON_SECURE)
    /*
     * This is only for DeveloperBox, set non-secure transaction
     * for PCIe#0/#1.
     */
    fw_set_sec_override(
        SEC_OVERRIDE_CONVERT_TO_NONSEC, SEC_OVERRIDE_REG_PCIEIF0_MMU);
    fw_set_sec_override(
        SEC_OVERRIDE_CONVERT_TO_NONSEC, SEC_OVERRIDE_REG_PCIEIF1_MMU);
#endif

    /* set Re-tunuing mode 0*/
    value = readl(CONFIG_SOC_SD_CTL_REG_BASE);

    value &= ~(0x3U << 8);

    writel(CONFIG_SOC_SD_CTL_REG_BASE, value);
}

void fw_clear_clkforce(uint32_t value)
{
    /*
     * Writing 1 to a bit enables dynamic hardware clock-gating.
     * Writing 0 to a bit is ignored.
     * bit8 : SYSPLLCLKDBGFORCE
     * bit7 : DMCCLKFORCE
     * bit6 : SYSPERCLKFORCE
     * bit5 : PCLKSCPFORCE
     * bit2 : CCNCLKFORCE
     * bit0 : PPUCLKFORCE
     */
    PIK_SYSTEM->CLKFORCE_CLR = value;
}

static crg11_state_t crg11_state[CONFIG_SOC_CRG11_NUM] = {
    {
        /* ID0 PERI */
        .ps_mode = 0,
        .fb_mode = 10U,
        .src_frequency = 25000000,
        .lcrg_numerator = INVALID_LCRG,
        .reg_addr = CONFIG_SOC_REG_ADDR_CRG_PERI_TOP,
        .clock_domain_div_modifiable_mask = 0x1fffU,
        .clock_domain_div = {
                0x3,
                0x3,
                0x7,
                0xf,
                0x1,
                0x7,
                0xf,
                0x3,
                0xf9,
                0x1,
                0x1,
                0x3,
                0x7,
                0,
                0,
                0,
            },
        .port_gate = { 0x1,
                       0xc1,
                       0x1,
                       0xf,
                       0x3,
                       0x1,
                       0x1,
                       0x1,
                       0x1,
                       0xff,
                       0x7d,
                       0x6,
                       0x3,
                       0x0,
                       0x0,
                       0x0 },
        .gate_enable = { 0x1,
                         0xc1,
                         0x1,
                         0xf,
                         0x3,
                         0x1,
                         0x1,
                         0x1,
                         0x1,
                         0xff,
                         0x7d,
                         0x6,
                         0x3,
                         0x0,
                         0x0,
                         0x0 },
        .ref_count = 1,
        .ap_change_allowed_flag = false,
    },
    { /* ID1 EMMC */
        .ps_mode = 0, .fb_mode = 26U, .src_frequency = 25000000,
        .lcrg_numerator = INVALID_LCRG,
        .reg_addr = CONFIG_SOC_REG_ADDR_CRG_EMMC_TOP,
        .clock_domain_div_modifiable_mask = 0x1fU,
        .clock_domain_div = {
            0, 1, 1, 7, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        .port_gate = { 0 }, .gate_enable = { 0 }, .ref_count = 1,
        .ap_change_allowed_flag = false,
    }
};

int synquacer_shutdown(void)
{
    volatile uint32_t *gpio = (uint32_t *)CONFIG_SOC_AP_GPIO_BASE;
    int status;

    /* set PD[9] high to turn off the ATX power supply */
    gpio[5] |= 0x2; /* set GPIO direction to output */
    __DSB();
    status = synquacer_system_ctx.timer_api->delay(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        10);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[SYNQUACER SYSTEM] timer_api->delay error(%d).", status);
    }
    gpio[1] |= 0x2; /* set high */

    for (;;) {
        __WFI();
    }
}


void crg11_soft_reset(uint8_t crg11_id)
{
    writel(CRG11_CRSWR(crg11_state[crg11_id].reg_addr), 1);
}

static void fw_system_reset(void)
{
    crg11_soft_reset(CONFIG_SCB_CRG11_ID_PERI);

    for (;;) {
        __WFI();
    }
}

int synquacer_reboot_chip(void)
{
    struct bootctl_reg *bootctl =
        (struct bootctl_reg *)CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP;

    FWK_LOG_INFO("[SYNQUACER SYSTEM] HSSPI initialize start.");
    synquacer_system_ctx.qspi_api->init_csmode(qspi_id);
    synquacer_system_ctx.nor_api->reset(nor_id, 0);

    /* configure BOOT_CTL as initial state */
    bootctl->BOOT_HSSPI |= (0x1U << 1);
    bootctl->BOOT_HSSPI &= ~(0x1U << 0);
    FWK_LOG_INFO("[SYNQUACER SYSTEM] HSSPI initialize end.");

    __disable_fault_irq();

    /* set default memory remap */
    set_memory_remap(8U);

    /* do chip reboot */
    fw_system_reset();

    /* Unreachable */

    return FWK_SUCCESS;
}

static void ap_dev_init(void)
{
    fw_ap_data_clk_preparation();
    fw_gpio_init();
    scb_am_block_init();
}

void main_initialize(void)
{
    int status = FWK_SUCCESS;
    (void)status;

    sysdef_option_init_synquacer();

    fw_clear_clkforce(sysdef_option_get_clear_clkforce());

    FWK_LOG_INFO(
        "[SYNQUACER SYSTEM] chip version %s.",
        sysdef_option_get_chip_version());

    power_domain_coldboot();
    nic_secure_access_ctrl_init();
    thermal_enable();

    __enable_irq();
    __enable_fault_irq();

    synquacer_system_ctx.f_i2c_api->init();

    status = fw_ddr_spd_param_check();
    fwk_assert(status == FWK_SUCCESS);

    /* read the eeprom configuration data */
    synquacer_system_ctx.nor_api->read(
        nor_id,
        0,
        MOD_NOR_READ_FAST_1_4_4_4BYTE,
        CONFIG_SCP_CONFIG_TABLE_OFFSET + EEPROM_CONFIG_T_START_OFFSET,
        &eeprom_config,
        sizeof(eeprom_config));
    bus_sysoc_init();

    return;
}

/* FWU platform metadata for SynQuacer */
struct fwu_synquacer_metadata {
    uint32_t boot_index;
    uint32_t boot_count;
} __attribute__((__packed__));

static void update_platform_metadata(struct fwu_synquacer_metadata *platdata)
{
    struct fwu_synquacer_metadata buf;

    synquacer_system_ctx.nor_api->read(nor_id, 0,
                                       MOD_NOR_READ_FAST_1_4_4_4BYTE,
                                       CONFIG_SCB_PLAT_METADATA_OFFS,
                                       &buf, sizeof(buf));

    if (!memcmp(platdata, &buf, sizeof(buf))) {
        return;
    }

    synquacer_system_ctx.nor_api->erase(nor_id, 0,
                                        MOD_NOR_ERASE_BLOCK_4BYTE,
                                        CONFIG_SCB_PLAT_METADATA_OFFS,
                                        sizeof(*platdata));
    synquacer_system_ctx.nor_api->program(nor_id, 0,
                                          MOD_NOR_PROGRAM_4BYTE,
                                          CONFIG_SCB_PLAT_METADATA_OFFS,
                                          platdata, sizeof(*platdata));
    /* Read to verify and set the "read" command-sequence */
    synquacer_system_ctx.nor_api->read(nor_id, 0,
                                       MOD_NOR_READ_FAST_1_4_4_4BYTE,
                                       CONFIG_SCB_PLAT_METADATA_OFFS,
                                       &buf, sizeof(buf));
    if (memcmp(platdata, &buf, sizeof(buf))) {
        FWK_LOG_ERR("[FWU] Failed to update boot-index!");
    }
}

static uint32_t fwu_plat_get_boot_index(void)
{
    struct fwu_synquacer_metadata platdata;
    struct fwu_mdata metadata;

    /* Read metadata */
    synquacer_system_ctx.nor_api->read(nor_id, 0,
                                       MOD_NOR_READ_FAST_1_4_4_4BYTE,
                                       CONFIG_SCB_FWU_METADATA_OFFS,
                                       &metadata, sizeof(metadata));

    synquacer_system_ctx.nor_api->read(nor_id, 0,
                                       MOD_NOR_READ_FAST_1_4_4_4BYTE,
                                       CONFIG_SCB_PLAT_METADATA_OFFS,
                                       &platdata, sizeof(platdata));

    /* TODO: use CRC32 */
    if (metadata.version != 1 ||
        metadata.active_index > CONFIG_FWU_NUM_BANKS) {
            platdata.boot_index = 0;
            FWK_LOG_ERR(
                "[FWU] FWU metadata is broken. Use default boot indx 0");
    } else if (metadata.img_entry[0].img_bank_info[metadata.active_index].accepted) {
        /* Image is accepted, skip trial boot */
        if (metadata.active_index != platdata.boot_index) {
            platdata.boot_index = metadata.active_index;
            platdata.boot_count = 0;
        } else {
            /* return here not to update metadata on every boot */
            return platdata.boot_index;
        }
    } else if (metadata.active_index != platdata.boot_index) {
        /* Switch to new active bank as a trial. */
        platdata.boot_index = metadata.active_index;
        platdata.boot_count = 1;
        FWK_LOG_INFO(
            "[FWU] New firmware will boot. New index is %d",
            (int)platdata.boot_index);
    } else if (platdata.boot_count) {
        /* BL33 will clear the boot_count when boot. */
        if (platdata.boot_count < CONFIG_FWU_MAX_COUNT) {
            platdata.boot_count++;
    } else {
            platdata.boot_index = metadata.previous_active_index;
            platdata.boot_count = 0;
            FWK_LOG_ERR(
                "[FWU] New firmware boot trial failed. Rollback index is %d",
                (int)platdata.boot_index);
        }
    }

    update_platform_metadata(&platdata);

    return platdata.boot_index;
}

static void fw_wakeup_ap(void)
{
    ap_dev_init();

    /* Check DSW 3-4 */
    if (gpio_get_data((void *)CONFIG_SOC_AP_GPIO_BASE, 0) & 0x8) {
        FWK_LOG_INFO("[SYNQUACER SYSTEM] Arm tf BL2 load start.");
        fw_fip_load_bl2(fwu_plat_get_boot_index());
        FWK_LOG_INFO("[SYNQUACER SYSTEM] Arm tf BL2 load end.");
    } else {
        FWK_LOG_INFO("[SYNQUACER SYSTEM] Arm tf load start.");
        fw_fip_load_arm_tf();
        FWK_LOG_INFO("[SYNQUACER SYSTEM] Arm tf load end.");
    }
    synquacer_system_ctx.nor_api->configure_mmap_read(
        nor_id, 0, MOD_NOR_READ_FAST_1_4_4_4BYTE, true);
}

int synquacer_main(void)
{
    int status;

    smmu_wrapper_initialize();
    pcie_wrapper_configure();
    fw_wakeup_ap();

    FWK_LOG_INFO("[SYNQUACER SYSTEM] powering up AP");
    status = synquacer_system_ctx.mod_pd_restricted_api->set_state(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
        MOD_PD_SET_STATE_NO_RESP,
        MOD_PD_COMPOSITE_STATE(
            MOD_PD_LEVEL_2,
            0,
            MOD_PD_STATE_ON,
            MOD_PD_STATE_ON,
            MOD_PD_STATE_ON));

    if (status == FWK_SUCCESS) {
        FWK_LOG_INFO("[SYNQUACER SYSTEM] finished powering up AP");
    } else {
        FWK_LOG_ERR(
            "[SYNQUACER SYSTEM] failed to power up AP. status=%d", status);
    }

    return status;
}
