/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DMC-BING driver
 */

#include "morello_pik_system.h"
#include "morello_scp_pik.h"

#include <mod_clock.h>
#include <mod_dmc_bing.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
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

static struct mod_timer_api *timer_api;
static uint64_t usable_mem_size;

static int ddr_get_dimm_size(uint64_t *size)
{
    /* FVP has fixed memory size of 8GB */
    *size = 0x200000000ULL;
    return FWK_SUCCESS;
}

/* Memory Information API */

static int ddr_get_mem_size(uint64_t *size)
{
    *size = usable_mem_size;
    return FWK_SUCCESS;
}

struct mod_dmc_bing_mem_info_api ddr_mem_info_api = {
    .get_mem_size = ddr_get_mem_size,
};

/*
 * DMC-Bing interrupt handling functions
 */

static void dmc_bing_abort_recover(struct mod_dmc_bing_reg *dmc)
{
    uint32_t current_state;
    volatile uint32_t *dmc_abort = 0;

    current_state = dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_STATUS;
    /* Make sure we don't run this from ABORT or RECOVERY states */
    if ((current_state == DMC_BING_ABORTED_STATE) ||
        (current_state == DMC_BING_RECOVER_STATE)) {
        FWK_LOG_INFO(
            "[DDR] DMC generated abortable error from abort/recovery state");
        return;
    }

    dmc_abort = (uint32_t *)((uint32_t)dmc + DMC_BING_ABORT_REG_OFFSET);

    /* Assert abort request */
    FWK_LOG_INFO("[DDR] Asserting abort request");
    *dmc_abort = ASSERT_ABORT_REQUEST;

    /* Wait for DMC to enter aborted state */
    FWK_LOG_INFO("[DDR] Waiting for DMC to enter abort state...");
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_STATUS) !=
           DMC_BING_ABORTED_STATE)
        continue;

    FWK_LOG_INFO("[DDR] DONE");

    /* Deassert abort request */
    FWK_LOG_INFO("[DDR] Deasserting abort request");
    *dmc_abort = DEASSERT_ABORT_REQUEST;

    /* Send ABORT_CLR command to change to recovery mode. */
    FWK_LOG_INFO("[DDR] Sending abort clear");
    dmc->MEMC_CMD = MOD_DMC_BING_MEMC_CMD_ABORT_CLR;

    /* Wait for state transition to complete */
    FWK_LOG_INFO("[DDR] Waiting for DMC state transition...");
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_STATUS) !=
           DMC_BING_RECOVER_STATE)
        continue;

    FWK_LOG_INFO("[DDR] DONE");

    /* Go back to pre-error state */
    FWK_LOG_INFO("[DDR] Initiating state transition back to normal world");
    dmc->MEMC_CMD = current_state;

    /* Wait for state transition to complete */
    FWK_LOG_INFO("[DDR] Waiting for DMC state transition...");
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_STATUS) != current_state)
        continue;

    FWK_LOG_INFO("[DDR] DONE");

    FWK_LOG_INFO("[DDR] Resuming operation in state %" PRIu32, current_state);
}

static void dmc_bing_handle_interrupt(int dmc_elem_idx)
{
    struct mod_dmc_bing_reg *dmc;
    const struct mod_dmc_bing_element_config *element_config;

    element_config = fwk_module_get_data(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DMC_BING, dmc_elem_idx));
    dmc = (struct mod_dmc_bing_reg *)element_config->dmc_bing_base;

    dmc_bing_abort_recover(dmc);
    dmc->INTERRUPT_CLR = 0x3FF;
}

static void dmc0_misc_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 MISC overflow interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MISC_OFLOW_IRQ);
}

static void dmc0_err_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 error overflow interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_ERR_OFLOW_IRQ);
}

static void dmc0_ecc_err_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 ECC error interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_ECC_ERR_INT_IRQ);
}

static void dmc0_misc_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 misc access interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MISC_ACCESS_INT_IRQ);
}

static void dmc0_temp_event_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 temperature event interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_TEMPERATURE_EVENT_INT_IRQ);
}

static void dmc0_failed_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 failed access interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_FAILED_ACCESS_INT_IRQ);
}

static void dmc0_mgr_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 mgr interrupt!");
    dmc_bing_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MGR_INT_IRQ);
}

static void dmc1_misc_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 MISC overflow interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MISC_OFLOW_IRQ);
}

static void dmc1_err_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 error overflow interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_ERR_OFLOW_IRQ);
}

static void dmc1_ecc_err_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 ECC error interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_ECC_ERR_INT_IRQ);
}

static void dmc1_misc_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 misc access interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MISC_ACCESS_INT_IRQ);
}

static void dmc1_temp_event_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 temperature event interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_TEMPERATURE_EVENT_INT_IRQ);
}

static void dmc1_failed_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 failed access interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_FAILED_ACCESS_INT_IRQ);
}

static void dmc1_mgr_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 mgr interrupt!");
    dmc_bing_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MGR_INT_IRQ);
}

static int dmc_bing_config_interrupt(fwk_id_t ddr_id)
{
    unsigned int idx;

    idx = fwk_id_get_element_idx(ddr_id);
    FWK_LOG_INFO("[DDR] Configuring interrupts for DMC%d", idx);

    if (idx == 0) {
        fwk_interrupt_set_isr(DMCS0_MISC_OFLOW_IRQ, dmc0_misc_oflow_handler);
        fwk_interrupt_set_isr(DMCS0_ERR_OFLOW_IRQ, dmc0_err_oflow_handler);
        fwk_interrupt_set_isr(DMCS0_ECC_ERR_INT_IRQ, dmc0_ecc_err_handler);
        fwk_interrupt_set_isr(
            DMCS0_MISC_ACCESS_INT_IRQ, dmc0_misc_access_handler);
        fwk_interrupt_set_isr(
            DMCS0_TEMPERATURE_EVENT_INT_IRQ, dmc0_temp_event_handler);
        fwk_interrupt_set_isr(
            DMCS0_FAILED_ACCESS_INT_IRQ, dmc0_failed_access_handler);
        fwk_interrupt_set_isr(DMCS0_MGR_INT_IRQ, dmc0_mgr_handler);
        fwk_interrupt_clear_pending(DMCS0_MISC_OFLOW_IRQ);
        fwk_interrupt_clear_pending(DMCS0_ERR_OFLOW_IRQ);
        fwk_interrupt_clear_pending(DMCS0_ECC_ERR_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS0_MISC_ACCESS_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS0_TEMPERATURE_EVENT_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS0_FAILED_ACCESS_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS0_MGR_INT_IRQ);
        fwk_interrupt_enable(DMCS0_MISC_OFLOW_IRQ);
        fwk_interrupt_enable(DMCS0_ERR_OFLOW_IRQ);
        fwk_interrupt_enable(DMCS0_ECC_ERR_INT_IRQ);
        fwk_interrupt_enable(DMCS0_MISC_ACCESS_INT_IRQ);
        fwk_interrupt_enable(DMCS0_TEMPERATURE_EVENT_INT_IRQ);
        fwk_interrupt_enable(DMCS0_FAILED_ACCESS_INT_IRQ);
        fwk_interrupt_enable(DMCS0_MGR_INT_IRQ);
    } else if (idx == 1) {
        fwk_interrupt_set_isr(DMCS1_MISC_OFLOW_IRQ, dmc1_misc_oflow_handler);
        fwk_interrupt_set_isr(DMCS1_ERR_OFLOW_IRQ, dmc1_err_oflow_handler);
        fwk_interrupt_set_isr(DMCS1_ECC_ERR_INT_IRQ, dmc1_ecc_err_handler);
        fwk_interrupt_set_isr(
            DMCS1_MISC_ACCESS_INT_IRQ, dmc1_misc_access_handler);
        fwk_interrupt_set_isr(
            DMCS1_TEMPERATURE_EVENT_INT_IRQ, dmc1_temp_event_handler);
        fwk_interrupt_set_isr(
            DMCS1_FAILED_ACCESS_INT_IRQ, dmc1_failed_access_handler);
        fwk_interrupt_set_isr(DMCS1_MGR_INT_IRQ, dmc1_mgr_handler);
        fwk_interrupt_clear_pending(DMCS1_MISC_OFLOW_IRQ);
        fwk_interrupt_clear_pending(DMCS1_ERR_OFLOW_IRQ);
        fwk_interrupt_clear_pending(DMCS1_ECC_ERR_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS1_MISC_ACCESS_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS1_TEMPERATURE_EVENT_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS1_FAILED_ACCESS_INT_IRQ);
        fwk_interrupt_clear_pending(DMCS1_MGR_INT_IRQ);
        fwk_interrupt_enable(DMCS1_MISC_OFLOW_IRQ);
        fwk_interrupt_enable(DMCS1_ERR_OFLOW_IRQ);
        fwk_interrupt_enable(DMCS1_ECC_ERR_INT_IRQ);
        fwk_interrupt_enable(DMCS1_MISC_ACCESS_INT_IRQ);
        fwk_interrupt_enable(DMCS1_TEMPERATURE_EVENT_INT_IRQ);
        fwk_interrupt_enable(DMCS1_FAILED_ACCESS_INT_IRQ);
        fwk_interrupt_enable(DMCS1_MGR_INT_IRQ);
    } else {
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

/*
 * DMC-Bing configuration functions
 */

static bool dmc_bing_wait_condition(void *data)
{
    fwk_assert(data != NULL);

    struct mod_dmc_bing_wait_condition_data *wait_data =
        (struct mod_dmc_bing_wait_condition_data *)data;
    struct mod_dmc_bing_reg *dmc = (struct mod_dmc_bing_reg *)(wait_data->dmc);

    switch (wait_data->stage) {
    case DMC_BING_CONFIG_STAGE_TRAINING_MGR_ACTIVE:
        return ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_STATUS_MGR_ACTIVE) == 0);
    case DMC_BING_CONFIG_STAGE_TRAINING_M0_IDLE:
        return (
            (dmc->CHANNEL_STATUS & MOD_DMC_BING_CHANNEL_STATUS_M0_IDLE) != 0);
    default:
        fwk_assert(false);
        return false;
    }
}

static int dmc_bing_poll_dmc_status(struct mod_dmc_bing_reg *dmc)
{
    struct mod_dmc_bing_wait_condition_data wait_data;

    wait_data.dmc = dmc;
    wait_data.stage = DMC_BING_CONFIG_STAGE_TRAINING_MGR_ACTIVE;
    return timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        DMC_TRAINING_TIMEOUT,
        dmc_bing_wait_condition,
        &wait_data);
}

static int ddr_poll_training_status(struct mod_dmc_bing_reg *dmc)
{
    struct mod_dmc_bing_wait_condition_data wait_data;
    int status;

    wait_data.dmc = dmc;
    wait_data.stage = DMC_BING_CONFIG_STAGE_TRAINING_MGR_ACTIVE;
    status = timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        DMC_TRAINING_TIMEOUT,
        dmc_bing_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[DDR] FAIL");
        return status;
    }

    wait_data.stage = DMC_BING_CONFIG_STAGE_TRAINING_M0_IDLE;
    status = timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        DMC_TRAINING_TIMEOUT,
        dmc_bing_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[DDR] FAIL");
        return status;
    }

    FWK_LOG_INFO("[DDR] PASS");

    return FWK_SUCCESS;
}

static int enable_dimm_refresh(struct mod_dmc_bing_reg *dmc)
{
    dmc->REFRESH_ENABLE_NEXT = 0x00000001;
    dmc->DIRECT_CMD = ((1U << 16) | 0x000C);

    return ddr_poll_training_status(dmc);
}

static void delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    while (ms > 0) {
        for (i = 0; i < 6000; i++)
            ;
        ms--;
    }
}

static void execute_ddr_cmd(
    struct mod_dmc_bing_reg *dmc,
    uint32_t addr,
    uint32_t cmd,
    uint8_t ms)
{
    int status;

    dmc->DIRECT_ADDR = addr;
    dmc->DIRECT_CMD = cmd;

    if (ms != 0)
        delay_ms(ms);

    status = dmc_bing_poll_dmc_status(dmc);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Execute command failed! ADDR: 0x%" PRIX32 " CMD: 0x%" PRIX32,
            addr,
            cmd);
    }
}

static int direct_ddr_cmd(struct mod_dmc_bing_reg *dmc)
{
    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

    execute_ddr_cmd(dmc, 0x00000004, 0x0001000A, 0);
    execute_ddr_cmd(dmc, 0x00000006, 0x00ff0004, 0);
    execute_ddr_cmd(dmc, 0x00000000, 0x0001000B, 0);
    execute_ddr_cmd(dmc, 0x00000001, 0x00ff000b, 0);

    execute_ddr_cmd(dmc, 0x000003E8, 0x0001000D, 0);
    execute_ddr_cmd(dmc, 0x00000258, 0x0001000D, 0);
    execute_ddr_cmd(dmc, 0x00010001, 0x00ff000b, 0);
    execute_ddr_cmd(dmc, 0x00000038, 0x0001000D, 0);

    execute_ddr_cmd(dmc, 0x00000000, 0x00ff0000, 0);
    dmc->DIRECT_ADDR = 0x00000038;
    execute_ddr_cmd(dmc, 0x00000220, 0x00ff0301, 0);
    execute_ddr_cmd(dmc, 0x00000800, 0x00ff0601, 0);
    execute_ddr_cmd(dmc, 0x00000600, 0x00ff0501, 0);

    execute_ddr_cmd(dmc, 0x00000000, 0x30ff0401, 0);
    execute_ddr_cmd(dmc, 0x00000018, 0x00ff0201, 0);
    execute_ddr_cmd(dmc, 0x00000001, 0x00ff0101, 0);
    execute_ddr_cmd(dmc, 0x00000940, 0x00ff0001, 0);

    execute_ddr_cmd(dmc, 0x00000300, 0x0001000d, 0);
    execute_ddr_cmd(dmc, 0x00000400, 0x00ff0005, 0);
    execute_ddr_cmd(dmc, 0x00000000, 0x0001000d, 0);

    return FWK_SUCCESS;
}

static int dmc_bing_config(struct mod_dmc_bing_reg *dmc, fwk_id_t ddr_id)
{
    uint64_t mem_size;
    int status;

    FWK_LOG_INFO("[DDR] Initialising DMC-Bing at 0x%x", (uintptr_t)dmc);

    dmc_bing_config_interrupt(ddr_id);

    FWK_LOG_INFO("[DDR] Writing functional settings");

    dmc->ADDRESS_CONTROL_NEXT = 0x11040202;
    dmc->DECODE_CONTROL_NEXT = 0x001A3010;
    dmc->FORMAT_CONTROL = 0x3;
    dmc->ADDRESS_MAP_NEXT = 0x00000003;
    dmc->ADDRESS_SHUTTER_31_00_NEXT = 0x11111110;
    dmc->ADDRESS_SHUTTER_63_32_NEXT = 0x11111111;
    dmc->ADDRESS_SHUTTER_95_64_NEXT = 0x11111111;
    dmc->ADDRESS_SHUTTER_127_96_NEXT = 0x11111111;
    dmc->ADDRESS_SHUTTER_159_128_NEXT = 0x11111111;
    dmc->ADDRESS_SHUTTER_191_160_NEXT = 0x00000011;
    dmc->LOW_POWER_CONTROL_NEXT = 0x00000010;
    dmc->MEMORY_ADDRESS_MAX_31_00_NEXT = 0xffff001f;
    dmc->MEMORY_ADDRESS_MAX_43_32_NEXT = 0x0000ffff;
    dmc->ACCESS_ADDRESS_NEXT[0].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[1].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[2].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[3].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[4].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[5].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[6].MIN_31_00 = 0x0000000F;
    dmc->ACCESS_ADDRESS_NEXT[7].MIN_31_00 = 0x0000000F;
    dmc->DCI_REPLAY_TYPE_NEXT = 0x00000000;
    dmc->DIRECT_CONTROL_NEXT = 0x00000000;
    dmc->DCI_STRB = 0x00000000;
    dmc->MEMORY_TYPE_NEXT = 0x00030002;
    dmc->FEATURE_CONFIG = 0x00001800;

    dmc->T_REFI_NEXT = 0x90000491;
    dmc->T_RFC_NEXT = 0x000691A4;
    dmc->T_MRR_NEXT = 0x00000001;
    dmc->T_MRW_NEXT = 0x00000018;
    dmc->T_RCD_NEXT = 0x00000012;
    dmc->T_RAS_NEXT = 0x00000027;
    dmc->T_RP_NEXT = 0x00000012;
    dmc->T_RPALL_NEXT = 0x00000012;
    dmc->T_RRD_NEXT = 0x04000604;
    dmc->T_ACT_WINDOW_NEXT = 0x0000101a;
    dmc->T_RTR_NEXT = 0x14060604;
    dmc->T_RTW_NEXT = 0x000c0c0c;
    dmc->T_RTP_NEXT = 0x00000009;
    dmc->T_WR_NEXT = 0x00000022;
    dmc->T_WTR_NEXT = 0x00051913;
    dmc->T_WTW_NEXT = 0x14060604;
    dmc->T_CLOCK_CONTROL_NEXT = 0x00000009;

    dmc->T_XMPD_NEXT = 0x000004af;
    dmc->T_EP_NEXT = 0x00000006;
    dmc->T_XP_NEXT = 0x00120008;
    dmc->T_ESR_NEXT = 0x00000007;
    dmc->T_XSR_NEXT = 0x030001af;
    dmc->T_ESRCK_NEXT = 0x0000000b;
    dmc->T_CKXSR_NEXT = 0x0000000b;
    dmc->T_PARITY_NEXT = 0x00001200;
    dmc->T_ZQCS_NEXT = 0x00000090;

    dmc->T_RDDATA_EN_NEXT = 0x00000000;
    dmc->T_PHYRDLAT_NEXT = 0x00000016;
    dmc->T_PHYWRLAT_NEXT = 0x001F000A;

    dmc->RDLVL_CONTROL_NEXT = 0x00000000;
    dmc->RDLVL_MRS_NEXT = 0x00000224;
    dmc->T_RDLVL_RR_NEXT = 0x00000018;
    dmc->WRLVL_CONTROL_NEXT = 0x00100000;
    dmc->WRLVL_MRS_NEXT = 0x00000181;
    dmc->T_WRLVL_EN_NEXT = 0x00000018;
    dmc->PHY_UPDATE_CONTROL_NEXT = 0x20003333;
    dmc->T_LVL_DISCONNECT_NEXT = 0x00000001;
    dmc->WDQLVL_CONTROL_NEXT = 0x00000080;
    dmc->T_WDQLVL_EN_NEXT = 0x00000027;
    dmc->T_WDQLVL_RW_NEXT = 0x0000000C;
    dmc->RANK_REMAP_CONTROL_NEXT = 0x76543210;
    dmc->PHY_REQUEST_CS_REMAP = 0x76543210;
    dmc->T_ODTH_NEXT = 0x00000006;
    dmc->ODT_TIMING_NEXT = 0x06000600;
    dmc->T_RW_ODT_CLR_NEXT = 0x00000010;
    dmc->T_CMD_NEXT = 0x00000000;
    dmc->T_RDLVL_EN_NEXT = 0x00000001;
    dmc->T_WRLVL_WW_NEXT = 0x00000001;
    dmc->PHYMSTR_CONTROL_NEXT = 0x00000000;
    dmc->T_LPRESP_NEXT = 0x00000000;
    dmc->ODT_WR_CONTROL_31_00_NEXT = 0x08040201;
    dmc->ODT_WR_CONTROL_63_32_NEXT = 0x80402010;
    dmc->ODT_RD_CONTROL_31_00_NEXT = 0x00000000;
    dmc->ODT_RD_CONTROL_63_32_NEXT = 0x00000000;
    dmc->ODT_CP_CONTROL_31_00_NEXT = 0x08040201;
    dmc->ODT_CP_CONTROL_63_32_NEXT = 0x80402010;

    dmc->POWER_GROUP_CONTROL_31_00_NEXT = 0x00020001;
    dmc->POWER_GROUP_CONTROL_63_32_NEXT = 0x00080004;
    dmc->POWER_GROUP_CONTROL_95_64_NEXT = 0x00200010;
    dmc->POWER_GROUP_CONTROL_127_96_NEXT = 0x00800040;
    dmc->REFRESH_CONTROL_NEXT = 0x00000000;
    dmc->T_DB_TRAIN_RESP_NEXT = 0x00000004;
    dmc->FEATURE_CONTROL_NEXT = 0x0aa00000;
    dmc->MUX_CONTROL_NEXT = 0x00000000;
    dmc->LOW_POWER_CONTROL_NEXT = 0x00000010;
    dmc->MEMORY_ADDRESS_MAX_31_00_NEXT = 0xffff001f;
    dmc->INTERRUPT_CONTROL = 0x00000070;
    dmc->ERR0CTLR0 = 0x00000400;
    dmc->ERR0CTLR1 = 0x000000C0;

    ddr_get_dimm_size(&mem_size);

    dmc->DIRECT_CMD = 0x0001000C;

    dmc->USER_CONFIG0_NEXT = 0x1;
    dmc->MEMC_CMD = MOD_DMC_BING_MEMC_CMD_GO;
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_CMD) !=
           MOD_DMC_BING_MEMC_CMD_GO)
        continue;

    dmc->MEMC_CMD = MOD_DMC_BING_MEMC_CMD_CONFIG;
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_CMD) !=
           MOD_DMC_BING_MEMC_CMD_CONFIG)
        continue;
    dmc->USER_CONFIG0_NEXT = 0x3;
    dmc->MEMC_CMD = MOD_DMC_BING_MEMC_CMD_GO;
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_CMD) !=
           MOD_DMC_BING_MEMC_CMD_GO)
        continue;
    dmc->MEMC_CMD = MOD_DMC_BING_MEMC_CMD_CONFIG;
    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_CMD) !=
           MOD_DMC_BING_MEMC_CMD_CONFIG)
        continue;

    FWK_LOG_INFO("[DDR] Sending direct DDR commands");

    direct_ddr_cmd(dmc);

    FWK_LOG_INFO("[DDR] Enable DIMM refresh...");
    status = enable_dimm_refresh(dmc);
    if (status != FWK_SUCCESS)
        return status;

    /* Switch to READY */
    FWK_LOG_INFO("[DDR] Setting DMC to READY mode");

    dmc->MEMC_CMD = MOD_DMC_BING_MEMC_CMD_GO;

    while ((dmc->MEMC_STATUS & MOD_DMC_BING_MEMC_CMD) !=
           MOD_DMC_BING_MEMC_CMD_GO)
        continue;

    FWK_LOG_INFO("[DDR] DMC init done.");

    return FWK_SUCCESS;
}

/* Framework API */
static int mod_dmc_bing_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *config)
{
    return FWK_SUCCESS;
}

static int mod_dmc_bing_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int mod_dmc_bing_bind(fwk_id_t id, unsigned int round)
{
    /* Nothing to do in the second round of calls. */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to do in case of elements. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    return fwk_module_bind(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
        &timer_api);
}

static int mod_dmc_bing_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_DDR_API_IDX_MEM_INFO:
        *api = &ddr_mem_info_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int mod_dmc_bing_start(fwk_id_t id)
{
    const struct mod_dmc_bing_element_config *element_config;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* Bypass divide by 2 so DMCCLK1X = DMCCLK2X */
        PIK_SYSTEM->DMCCLK_CTRL |= PIK_SYSTEM_DMCCLK_CTRL_DIV2_BYPASS_MASK;
        return FWK_SUCCESS;
    }

    element_config = fwk_module_get_data(id);

    /* Register elements for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed, element_config->clock_id, id);
}

static int dmc_bing_notify_system_state_transition_resume(fwk_id_t id)
{
    struct mod_dmc_bing_reg *dmc;
    const struct mod_dmc_bing_element_config *element_config;

    element_config = fwk_module_get_data(id);
    dmc = (struct mod_dmc_bing_reg *)element_config->dmc_bing_base;

    return dmc_bing_config(dmc, id);
}

static int mod_dmc_bing_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING)
        return dmc_bing_notify_system_state_transition_resume(event->target_id);

    return FWK_SUCCESS;
}

const struct fwk_module module_dmc_bing = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_dmc_bing_init,
    .element_init = mod_dmc_bing_element_init,
    .bind = mod_dmc_bing_bind,
    .start = mod_dmc_bing_start,
    .process_notification = mod_dmc_bing_process_notification,
    .process_bind_request = mod_dmc_bing_process_bind_request,
    .api_count = MOD_DDR_API_COUNT,
    .event_count = 0,
};
