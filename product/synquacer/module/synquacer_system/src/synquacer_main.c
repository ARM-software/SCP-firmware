/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "pik_system.h"
#include "synquacer_common.h"
#include "synquacer_config.h"
#include "synquacer_mmap.h"

#include <boot_ctl.h>
#include <sysdef_option.h>

#include <internal/crg11.h>
#include <internal/gpio.h>
#include <internal/nic400.h>
#include <internal/thermal_sensor.h>

#include <mod_f_i2c.h>
#include <mod_hsspi.h>
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

void power_domain_coldboot(void);
int fw_ddr_spd_param_check(void);
void bus_sysoc_init(void);
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
     * as a non-secure master
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

void crg11_soft_reset(uint8_t crg11_id)
{
    writel(CRG11_CRSWR(crg11_state[crg11_id].reg_addr), 1);
}

static void fw_system_reset(void)
{
    crg11_soft_reset(CONFIG_SCB_CRG11_ID_PERI);

    for (;;)
        __WFI();
}

int reboot_chip(void)
{
    FWK_LOG_INFO("[SYNQUACER SYSTEM] HSSPI exit start.");
    synquacer_system_ctx.hsspi_api->hsspi_exit();
    FWK_LOG_INFO("[SYNQUACER SYSTEM] HSSPI exit end.");

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
    assert(status == FWK_SUCCESS);

    /* prepare eeprom configuration data */
    memcpy(
        &eeprom_config,
        (char *)(CONFIG_SCP_CONFIG_TABLE_ADDR + EEPROM_CONFIG_T_START_OFFSET),
        sizeof(eeprom_config));

    bus_sysoc_init();

    return;
}

static void fw_wakeup_ap(void)
{
    ap_dev_init();

    synquacer_system_ctx.hsspi_api->hsspi_init();
    FWK_LOG_INFO("[SYNQUACER SYSTEM] Finished initializing HS-SPI controller.");
    FWK_LOG_INFO("[SYNQUACER SYSTEM] Arm tf load start.");
    fw_fip_load_arm_tf();
    FWK_LOG_INFO("[SYNQUACER SYSTEM] Arm tf load end.");
}

int synquacer_main(void)
{
    int status;

    smmu_wrapper_initialize();
    pcie_wrapper_configure();
    fw_wakeup_ap();

    FWK_LOG_INFO("[SYNQUACER SYSTEM] powering up AP");
    status =
        synquacer_system_ctx.mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
            false,
            MOD_PD_COMPOSITE_STATE(
                MOD_PD_LEVEL_2,
                0,
                MOD_PD_STATE_ON,
                MOD_PD_STATE_ON,
                MOD_PD_STATE_ON));

    if (status == FWK_SUCCESS)
        FWK_LOG_INFO("[SYNQUACER SYSTEM] finished powering up AP");
    else
        FWK_LOG_ERR(
            "[SYNQUACER SYSTEM] failed to power up AP. status=%d", status);

    return status;
}
