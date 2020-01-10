/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP DMC-620 driver
 */

#include "n1sdp_pik_system.h"
#include "n1sdp_scp_irq.h"
#include "n1sdp_scp_pik.h"

#include <dimm_spd.h>

#include <mod_clock.h>
#include <mod_n1sdp_dmc620.h>
#include <mod_n1sdp_i2c.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

/* DMC-620 register specific definitions */
#define DDR_TRAIN_TWO_RANKS          0

static struct mod_dmc_ddr_phy_api *ddr_phy_api;
static struct mod_timer_api *timer_api;
static struct mod_n1sdp_i2c_master_api_polled *i2c_api;
static struct dimm_info ddr_info;

/*
 * DMC-620 interrupt handling functions
 */

void dmc620_abort_recover(struct mod_dmc620_reg *dmc)
{
    uint32_t current_state;
    volatile uint32_t *dmc_abort = 0;

    current_state = dmc->MEMC_STATUS & 0x00000007;
    /* Make sure we don't run this from ABORT or RECOVERY states */
    if (current_state > 3) {
        FWK_LOG_INFO(
            "[DDR] DMC generated abortable error from abort/recovery state");
        return;
    }

    /* Abort register is at offset 0x10000 */
    dmc_abort = (uint32_t *)((uint32_t)dmc + 0x10000);

    /* Assert abort request */
    FWK_LOG_INFO("[DDR] Asserting abort request");
    *dmc_abort = 0x1;

    /* Wait for DMC to enter aborted state */
    FWK_LOG_INFO("[DDR] Waiting for DMC to enter abort state...");
    while ((dmc->MEMC_STATUS & 0x00000007) != 0x4)
        continue;

    FWK_LOG_INFO("[DDR] DONE");

    /* Deassert abort request */
    FWK_LOG_INFO("[DDR] Deasserting abort request");
    *dmc_abort = 0x0;

    /* Send ABORT_CLR command to change to recovery mode. */
    FWK_LOG_INFO("[DDR] Sending abort clear");
    dmc->MEMC_CMD = 0x00000006;

    /* Wait for state transition to complete */
    FWK_LOG_INFO("[DDR] Waiting for DMC state transition...");
    while ((dmc->MEMC_STATUS & 0x00000007) != 0x5)
        continue;

    FWK_LOG_INFO("[DDR] DONE");

    /* Go back to pre-error state */
    FWK_LOG_INFO("[DDR] Initiating state transition back to normal world");
    dmc->MEMC_CMD = current_state;

    /* Wait for state transition to complete */
    FWK_LOG_INFO("[DDR] Waiting for DMC state transition...");
    while ((dmc->MEMC_STATUS & 0x00000007) != current_state)
        continue;

    FWK_LOG_INFO("[DDR] DONE");

    FWK_LOG_INFO("[DDR] Resuming operation in state %" PRIu32, current_state);
}

void dmc620_handle_interrupt(int dmc_num)
{
    struct mod_dmc620_reg *dmc;
    const struct mod_dmc620_element_config *element_config;

    element_config = fwk_module_get_data(
                         FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_DMC620, dmc_num));
    dmc = (struct mod_dmc620_reg *)element_config->dmc;

    dmc620_abort_recover(dmc);
    dmc->INTERRUPT_CLR = 0x3FF;
}

void dmc0_misc_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 MISC overflow interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MISC_OFLOW_IRQ);
}

void dmc0_err_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 error overflow interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_ERR_OFLOW_IRQ);
}

void dmc0_ecc_err_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 ECC error interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_ECC_ERR_INT_IRQ);
}

void dmc0_misc_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 misc access interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MISC_ACCESS_INT_IRQ);
}

void dmc0_temp_event_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 temperature event interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_TEMPERATURE_EVENT_INT_IRQ);
}

void dmc0_failed_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 failed access interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_FAILED_ACCESS_INT_IRQ);
}

void dmc0_mgr_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC0 mgr interrupt!");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MGR_INT_IRQ);
}

void dmc1_misc_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 MISC overflow interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MISC_OFLOW_IRQ);
}

void dmc1_err_oflow_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 error overflow interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_ERR_OFLOW_IRQ);
}

void dmc1_ecc_err_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 ECC error interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_ECC_ERR_INT_IRQ);
}

void dmc1_misc_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 misc access interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MISC_ACCESS_INT_IRQ);
}

void dmc1_temp_event_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 temperature event interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_TEMPERATURE_EVENT_INT_IRQ);
}

void dmc1_failed_access_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 failed access interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_FAILED_ACCESS_INT_IRQ);
}

void dmc1_mgr_handler(void)
{
    FWK_LOG_INFO("[DDR] DMC1 mgr interrupt!");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MGR_INT_IRQ);
}

static int dmc620_config_interrupt(fwk_id_t ddr_id)
{
    int id;

    id = fwk_id_get_element_idx(ddr_id);
    FWK_LOG_INFO("[DDR] Configuring interrupts for DMC%d", id);

    if (id == 0) {
        fwk_interrupt_set_isr(DMCS0_MISC_OFLOW_IRQ, dmc0_misc_oflow_handler);
        fwk_interrupt_set_isr(DMCS0_ERR_OFLOW_IRQ, dmc0_err_oflow_handler);
        fwk_interrupt_set_isr(DMCS0_ECC_ERR_INT_IRQ, dmc0_ecc_err_handler);
        fwk_interrupt_set_isr(DMCS0_MISC_ACCESS_INT_IRQ,
            dmc0_misc_access_handler);
        fwk_interrupt_set_isr(DMCS0_TEMPERATURE_EVENT_INT_IRQ,
            dmc0_temp_event_handler);
        fwk_interrupt_set_isr(DMCS0_FAILED_ACCESS_INT_IRQ,
            dmc0_failed_access_handler);
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
    } else if (id == 1) {
        fwk_interrupt_set_isr(DMCS1_MISC_OFLOW_IRQ, dmc1_misc_oflow_handler);
        fwk_interrupt_set_isr(DMCS1_ERR_OFLOW_IRQ, dmc1_err_oflow_handler);
        fwk_interrupt_set_isr(DMCS1_ECC_ERR_INT_IRQ, dmc1_ecc_err_handler);
        fwk_interrupt_set_isr(DMCS1_MISC_ACCESS_INT_IRQ,
            dmc1_misc_access_handler);
        fwk_interrupt_set_isr(DMCS1_TEMPERATURE_EVENT_INT_IRQ,
            dmc1_temp_event_handler);
        fwk_interrupt_set_isr(DMCS1_FAILED_ACCESS_INT_IRQ,
            dmc1_failed_access_handler);
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
 * DMC-620 configuration functions
 */

bool dmc620_wait_condition(void *data)
{
    fwk_assert(data != NULL);

    struct dmc620_wait_condition_data *wait_data =
        (struct dmc620_wait_condition_data *)data;
    struct mod_dmc620_reg *dmc =
        (struct mod_dmc620_reg *)(wait_data->dmc);

    switch (wait_data->stage) {
    case DMC620_CONFIG_STAGE_TRAINING_MGR_ACTIVE:
        return ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_STATUS_MGR_ACTIVE) == 0);
    case DMC620_CONFIG_STAGE_TRAINING_M0_IDLE:
        return ((dmc->CHANNEL_STATUS &
                 MOD_DMC620_CHANNEL_STATUS_M0_IDLE) != 0);
    default:
        fwk_assert(false);
        return false;
    }
}

static int ddr_poll_training_status(struct mod_dmc620_reg *dmc)
{
    struct dmc620_wait_condition_data wait_data;
    int status;

    wait_data.dmc = dmc;
    wait_data.stage = DMC620_CONFIG_STAGE_TRAINING_MGR_ACTIVE;
    status = timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                             DMC_TRAINING_TIMEOUT,
                             dmc620_wait_condition,
                             &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[DDR] FAIL");
        return status;
    }

    wait_data.stage = DMC620_CONFIG_STAGE_TRAINING_M0_IDLE;
    status = timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                             DMC_TRAINING_TIMEOUT,
                             dmc620_wait_condition,
                             &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[DDR] FAIL");
        return status;
    }

    FWK_LOG_INFO("[DDR] PASS");

    return FWK_SUCCESS;
}

static int dmc620_poll_dmc_status(struct mod_dmc620_reg *dmc)
{
    struct dmc620_wait_condition_data wait_data;

    wait_data.dmc = dmc;
    wait_data.stage = DMC620_CONFIG_STAGE_TRAINING_MGR_ACTIVE;
    return timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                           DMC_TRAINING_TIMEOUT,
                           dmc620_wait_condition,
                           &wait_data);
}

static int ddr_training(struct mod_dmc620_reg *dmc,
    fwk_id_t ddr_id, struct dimm_info *info)
{
    uint32_t value;
    int i;
    int j;
    int status;

    FWK_LOG_INFO("[DDR] Training DDR memories...");

    for (i = 1; i <= ddr_info.number_of_ranks; i++) {
        FWK_LOG_INFO("[DDR] Write leveling rank %d... ", i);

        /* Clear interrupt status if any */
        if (dmc->INTERRUPT_STATUS != 0)
            dmc->INTERRUPT_CLR = 0xFFFFFFFF;

        /* Set training command */
        dmc->DIRECT_ADDR = DDR_ADDR_TRAIN_TYPE_WR_LVL;
        if (dmc->DIRECT_ADDR != DDR_ADDR_TRAIN_TYPE_WR_LVL) {
            for (j = 1; j <= ddr_info.number_of_ranks; j++)
                ddr_phy_api->wrlvl_phy_obs_regs(ddr_id, j, info);
            return FWK_E_DEVICE;
        }
        dmc->DIRECT_CMD  = ((1 << (i + 15)) | 0x000A);
        status = ddr_poll_training_status(dmc);
        if (status != FWK_SUCCESS) {
            for (j = 1; j <= ddr_info.number_of_ranks; j++)
                ddr_phy_api->wrlvl_phy_obs_regs(ddr_id, j, info);
            return status;
        }
        ddr_phy_api->wrlvl_phy_obs_regs(ddr_id, i, info);
    }
    ddr_phy_api->verify_phy_status(ddr_id, DDR_ADDR_TRAIN_TYPE_WR_LVL, info);

    FWK_LOG_INFO("[DDR] Read gate training");
    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

    FWK_LOG_INFO("[DDR] A side...");

    /* Set read level control parameter */
    value = dmc->RDLVL_CONTROL_NEXT;
    value = (value & 0xFFFFF9FF) | (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;
    /* Update */
    dmc->DIRECT_ADDR = 0;
    value = ((ddr_info.ranks_to_train << 16) | 0x000C);
    dmc->DIRECT_CMD  = value;

    /* Run training on slices 0-9 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_GATE |
                        (0x3FFFF << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000A);

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS) {
        for (j = 1; j <= ddr_info.number_of_ranks; j++)
            ddr_phy_api->read_gate_phy_obs_regs(ddr_id, j, info);
        return status;
    }

    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

#if DDR_TRAIN_TWO_RANKS
    FWK_LOG_INFO("[DDR] B side...");

    /* Set write leveling parameters */
    value = dmc->RDLVL_CONTROL_NEXT;
    value |= (0x03 << 9) | (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;
    /* Update */
    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = 0x0001000C;

    /* Run training on slices 10-17 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_GATE |
                        (0x3FFFF << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = DDR_CMD_TRAIN_RANK_1;

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;
#endif

    for (j = 1; j <= ddr_info.number_of_ranks; j++)
        ddr_phy_api->read_gate_phy_obs_regs(ddr_id, j, info);

    FWK_LOG_INFO("[DDR] Read eye training");

    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

    FWK_LOG_INFO("[DDR] A side...");

    /* Set write leveling parameters */
    value = dmc->RDLVL_CONTROL_NEXT;
    value = (value & 0xFFFFF9FF) | (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;

    /* Update */
    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000C);

    /* Run training on slices 0-9 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_EYE |
                        (0x201FF << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000A);

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS) {
        for (j = 1; j <= ddr_info.number_of_ranks; j++)
            ddr_phy_api->phy_obs_regs(ddr_id, j, info);
        return status;
    }

    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

    FWK_LOG_INFO("[DDR] B side...");

    /* Set write leveling parameters */
    value = dmc->RDLVL_CONTROL_NEXT;
    value |= (0x03 << 9) | (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;
    /* Update */
    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000C);

    /* Run training on slices 10-17 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_EYE |
                        (0x1FE00 << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000A);

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS) {
        for (j = 1; j <= ddr_info.number_of_ranks; j++)
            ddr_phy_api->phy_obs_regs(ddr_id, j, info);
        return status;
    }

    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

    FWK_LOG_INFO("[DDR] MC initiated update...");

    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000A);

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000C);

    for (j = 1; j <= ddr_info.number_of_ranks; j++)
        ddr_phy_api->phy_obs_regs(ddr_id, j, info);

    return FWK_SUCCESS;
}

static int dmc620_verify_phy_status(fwk_id_t ddr_id)
{
    int status;

    status = ddr_phy_api->verify_phy_status(ddr_id,
        DDR_ADDR_TRAIN_TYPE_WR_LVL, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_phy_api->verify_phy_status(ddr_id,
        DDR_ADDR_TRAIN_TYPE_RD_GATE, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_phy_api->verify_phy_status(ddr_id,
        DDR_ADDR_TRAIN_TYPE_RD_EYE, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_phy_api->verify_phy_status(ddr_id,
        DDR_ADDR_TRAIN_TYPE_VREF, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static void delay_ms(uint32_t ms)
{
    volatile uint32_t i = 0;
    while (ms) {
        for (i = 0; i < 6000; i++)
            ;
        ms--;
    }
}

static void execute_ddr_cmd(struct mod_dmc620_reg *dmc,
    uint32_t addr, uint32_t cmd, uint8_t ms)
{
    int status;

    dmc->DIRECT_ADDR = addr;
    dmc->DIRECT_CMD = cmd;

    if (ms != 0)
        delay_ms(ms);

    status = dmc620_poll_dmc_status(dmc);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Execute command failed! ADDR: 0x%" PRIX32 " CMD: 0x%" PRIX32,
            addr,
            cmd);
    }
}

static int direct_ddr_cmd(struct mod_dmc620_reg *dmc)
{
    int count;
    uint32_t addr;
    int status;

    /* Clear interrupt status if any */
    if (dmc->INTERRUPT_STATUS != 0)
        dmc->INTERRUPT_CLR = 0xFFFFFFFF;

    execute_ddr_cmd(dmc, 0x00000004, 0x0001000A, 0);
    execute_ddr_cmd(dmc, 0x00000006,
                    ((ddr_info.ranks_to_train << 16) | 0x0004), 0);
    execute_ddr_cmd(dmc, 0x00000000, 0x0001000B, 0);
    execute_ddr_cmd(dmc, 0x00000001,
                    ((ddr_info.ranks_to_train << 16) | 0x000B), 0);
    execute_ddr_cmd(dmc, 0x000003E8, 0x0001000D, 0);
    execute_ddr_cmd(dmc, 0x00000258, 0x0001000D, 1);
    execute_ddr_cmd(dmc, 0x00010001,
                    ((ddr_info.ranks_to_train << 16) | 0x000B), 0);
    execute_ddr_cmd(dmc, 0x0000002A, 0x0001000D, 0);

    for (count = 0; count < 12; count++)
        execute_ddr_cmd(dmc, 0x00000200, 0x0001000D, 1);

    execute_ddr_cmd(dmc, 0x00000000,
                    ((ddr_info.ranks_to_train << 16) | 0x0000), 0);

    /* Setting RCD RC0A, before CKE goes HIGH */
    addr = 0;
    switch (ddr_info.speed) {
    case 800:
        addr = 0x000000A0;
        break;
    case 1200:
        addr = 0x000000A3;
        break;
    case 1333:
        addr = 0x000000A4;
        break;
    default:
        fwk_assert(false);
        break;
    }
    execute_ddr_cmd(dmc, addr, 0x0001070F, 5);

    /* Setting RC3x F0RC3x, before CKE goes HIGH, register for speed */
    addr = 0;
    switch (ddr_info.speed) {
    case 800:
        addr = 0x00000311;
        break;
    case 1200:
        addr = 0x00000339;
        break;
    case 1333:
        addr = 0x00000347;
        break;
    default:
        fwk_assert(false);
        break;
    }
    execute_ddr_cmd(dmc, addr, 0x0001070F, 5);

    /* RC0D, dual CS, RDIMM, mirroring disabled */
    execute_ddr_cmd(dmc, 0x000000DC, 0x0001070F, 0);

    /*
     * RC0E
     * parity control word, parity checking enabled, alert_n pulse width set
     */
    execute_ddr_cmd(dmc, 0x000000EC, 0x0001070F, 0);

    /* RC03, CA and CS Signals Driver Characteristics Control Word */
    addr = 0;
    if (ddr_info.number_of_ranks == 1) {
        if (ddr_info.dimm_mem_width == 4)
            addr = 0x00000035;
        else
            addr = 0x00000030;
    } else {
        if (ddr_info.dimm_mem_width == 4)
            addr = 0x0000003A;
        else
            addr = 0x00000035;
    }
    execute_ddr_cmd(dmc, addr, 0x0001070F, 5);

    /* RC04, ODT and CKE Signal Driver Characteristics Control Word */
    addr = 0;
    if (ddr_info.number_of_ranks == 1) {
        if (ddr_info.dimm_mem_width == 4)
            addr = 0x00000045;
        else
            addr = 0x00000040;
    } else {
        if (ddr_info.dimm_mem_width == 4)
            addr = 0x0000004A;
        else
            addr = 0x00000045;
    }
    execute_ddr_cmd(dmc, addr, 0x0001070F, 5);

    /* RC05, Clock Driver Characteristics Control Word */
    addr = 0;
    if (ddr_info.number_of_ranks == 1) {
        if (ddr_info.dimm_mem_width == 4)
            addr = 0x00000055;
        else
            addr = 0x00000050;
    } else {
        if (ddr_info.dimm_mem_width == 4)
            addr = 0x0000005A;
        else
            addr = 0x00000055;
    }
    execute_ddr_cmd(dmc, addr, 0x0001070F, 5);

    /* MRS3 */
    execute_ddr_cmd(dmc, 0x00000220,
                    ((ddr_info.ranks_to_train << 16) | 0x0301), 0);

    /* MRS6 */
    addr = 0;
    switch (ddr_info.speed) {
    case 800:
        if (ddr_info.number_of_ranks == 1)
            addr = 0x00000497;
        else
            addr = 0x000004A3;
        break;
    case 1200:
        if (ddr_info.number_of_ranks == 1)
            addr = 0x00000894;
        else
            addr = 0x000008A3;
        break;
    case 1333:
        if (ddr_info.number_of_ranks == 1)
            addr = 0x00000C95;
        else
            addr = 0x00000CA3;
        break;
    default:
        fwk_assert(false);
        break;
    }
    addr = addr | 0x00000080;
    execute_ddr_cmd(dmc, addr,
                    ((ddr_info.ranks_to_train << 16) | 0x0601), 0);

    delay_ms(1);
    dmc->DIRECT_CMD = ((ddr_info.ranks_to_train << 16) | 0x0601);
    status = dmc620_poll_dmc_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

    addr = addr & 0xFFFFFF7F;
    execute_ddr_cmd(dmc, addr,
                    ((ddr_info.ranks_to_train << 16) | 0x0601), 1);

    /* MRS5 */
    execute_ddr_cmd(dmc, 0x00000180,
                    ((ddr_info.ranks_to_train << 16) | 0x0501), 0);

    /* MRS4 */
    execute_ddr_cmd(dmc, 0x00000000,
                    ((ddr_info.ranks_to_train << 16) | 0x0401), 0);

    /* MRS2 */
    addr = 0;
    switch (ddr_info.speed) {
    case 800:
        addr = 0x00000800;
        ddr_info.cwl_value = 9;
        break;
    case 1200:
        addr = 0x00000818;
        ddr_info.cwl_value = 12;
        break;
    case 1333:
        addr = 0x00000820;
        ddr_info.cwl_value = 14;
        break;
    default:
        fwk_assert(false);
        break;
    }
    execute_ddr_cmd(dmc, addr,
                    ((ddr_info.ranks_to_train << 16) | 0x0201), 0);

    /* MRS1 */
    execute_ddr_cmd(dmc, 0x00000003,
                    ((ddr_info.ranks_to_train << 16) | 0x0101), 0);

    /* MRS0 */
    addr = 0;
    switch (ddr_info.speed) {
    case 800:
        addr = 0x00000B10;
        break;
    case 1200:
        addr = 0x00000B40;
        break;
    case 1333:
        addr = 0x00000B44;
        break;
    default:
        fwk_assert(false);
        break;
    }
    execute_ddr_cmd(dmc, addr,
                    ((ddr_info.ranks_to_train << 16) | 0x0001), 0);

    addr = 0;
    status = dimm_spd_t_wtr(&addr, &ddr_info);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_WTR value from SPD", status);
        return status;
    }
    dmc->T_WTR_NEXT = addr;

    for (count = 0; count < 12; count++)
        execute_ddr_cmd(dmc, 0x00000200, 0x0001000D, 0);

    execute_ddr_cmd(dmc, 0x00000400,
                    ((ddr_info.ranks_to_train << 16) | 0x0005), 0);

    for (count = 0; count < 12; count++)
        execute_ddr_cmd(dmc, 0x00000200, 0x0001000D, 0);

    return FWK_SUCCESS;
}

static int enable_dimm_refresh(struct mod_dmc620_reg *dmc)
{
    dmc->REFRESH_ENABLE_NEXT = 0x00000001;
    dmc->DIRECT_CMD  = ((ddr_info.ranks_to_train << 16) | 0x000C);

    return ddr_poll_training_status(dmc);
}

static int dmc620_pre_init(void)
{
    int status;

    FWK_LOG_INFO(
        "[DDR] Starting DDR subsystem initialization at %d MHz",
        ddr_info.speed);

    FWK_LOG_INFO("[DDR] Identifying connected DIMM cards...");
    status = dimm_spd_init_check(i2c_api, &ddr_info);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[DDR] Error checking DIMM SPD data: %d", status);
        return status;
    }

    dimm_spd_mem_info();

    return FWK_SUCCESS;
}

static int dmc620_post_init(void)
{
    int status;
    int i;
    int j;
    int count;
    fwk_id_t id;
    const struct mod_dmc620_element_config *element_config;

    count = fwk_module_get_element_count(
        FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_DMC620));
    for (i = 0; i < count; i++) {
        id = FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_DMC620, i);
        element_config = fwk_module_get_data(id);

        FWK_LOG_INFO("[DDR] Verifying PHY status for DMC %d...", i);
        status = dmc620_verify_phy_status(element_config->ddr_id);
        if (status != FWK_SUCCESS)
            return status;
        FWK_LOG_INFO("[DDR] Done");
    }

    for (i = 0; i < count; i++) {
        id = FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_DMC620, i);
        element_config = fwk_module_get_data(id);
        for (j = 1; j <= ddr_info.number_of_ranks; j++)
            ddr_phy_api->phy_obs_regs(element_config->ddr_id, j, &ddr_info);
    }

    return FWK_SUCCESS;
}

static int dmc620_config(struct mod_dmc620_reg *dmc, fwk_id_t ddr_id)
{
    int status;
    int dmc_id;
    uint32_t value;

    dmc_id = fwk_id_get_element_idx(ddr_id);
    if (dmc_id == 0) {
        status = dmc620_pre_init();
        if (status != FWK_SUCCESS)
            return status;
    }

    FWK_LOG_INFO("[DDR] Initialising DMC620 at 0x%x", (uintptr_t)dmc);

    dmc620_config_interrupt(ddr_id);

    FWK_LOG_INFO("[DDR] Writing functional settings");

    value = 0;
    status = dimm_spd_address_control(&value, &ddr_info);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting address control value from SPD",
            status);
        return status;
    }
    dmc->ADDRESS_CONTROL_NEXT = value | DMC_ADDR_CTLR_BANK_HASH_ENABLE;
    dmc->DECODE_CONTROL_NEXT = 0x00142C10;

    value = 0;
    status = dimm_spd_format_control(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting format control value from SPD",
            status);
        return status;
    }
    dmc->FORMAT_CONTROL = value;
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

    value = 0;
    status = dimm_spd_memory_type(&value, &ddr_info);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting memory type value from SPD", status);
        return status;
    }
    dmc->MEMORY_TYPE_NEXT = value;
    dmc->FEATURE_CONFIG = 0x00001820;

    value = 0;
    status = dimm_spd_t_refi(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_REFI value from SPD", status);
        return status;
    }
    dmc->T_REFI_NEXT = value;

    value = 0;
    status = dimm_spd_t_rfc(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_RFC value from SPD", status);
        return status;
    }
    dmc->T_RFC_NEXT = value;
    dmc->T_MRR_NEXT = 0x00000001;
    dmc->T_MRW_NEXT = 0x00080030;

    value = 0;
    status = dimm_spd_t_rcd(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_RCD value from SPD", status);
        return status;
    }
    dmc->T_RCD_NEXT = value;

    value = 0;
    status = dimm_spd_t_ras(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_RAS value from SPD", status);
        return status;
    }
    dmc->T_RAS_NEXT = value;

    value = 0;
    status = dimm_spd_t_rp(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[DDR] Error code %d getting t_RP value from SPD", status);
        return status;
    }
    dmc->T_RP_NEXT = value;
    dmc->T_RPALL_NEXT = 0x00000013;

    value = 0;
    status = dimm_spd_t_rrd(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_RRD value from SPD", status);
        return status;
    }
    dmc->T_RRD_NEXT = value;

    value = 0;
    status = dimm_spd_t_act_window(&value);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[DDR] Error code %d getting t_ACT_WINDOW value from SPD", status);
        return status;
    }
    dmc->T_ACT_WINDOW_NEXT = value;

    if ((ddr_info.speed == 1333) || (ddr_info.speed == 1200))
        dmc->T_RTR_NEXT = 0x24090704;
    else
        dmc->T_RTR_NEXT = 0x14060604;

    dmc->T_RTW_NEXT = 0x001B1B1B;
    dmc->T_RTP_NEXT = 0x00000008;
    dmc->T_WR_NEXT = 0x00000029;
    dmc->T_WTR_NEXT = 0x001B1B1B;

    if ((ddr_info.speed == 1333) || (ddr_info.speed == 1200))
        dmc->T_WTW_NEXT = 0x24090704;
    else
        dmc->T_WTW_NEXT = 0x14060604;

    dmc->T_XMPD_NEXT = 0x00000480;
    dmc->T_EP_NEXT = 0x00000006;
    dmc->T_XP_NEXT = 0x000e0007;
    dmc->T_ESR_NEXT = 0x00000007;
    dmc->T_XSR_NEXT = 0x03000384;
    dmc->T_ESRCK_NEXT = 0x0000000a;
    dmc->T_CKXSR_NEXT = 0x0000000a;
    dmc->T_PARITY_NEXT = 0x00001100;
    dmc->T_ZQCS_NEXT = 0x00000090;

    switch (ddr_info.speed) {
    case 800:
        dmc->T_RDDATA_EN_NEXT = 0x00070007;
        break;
    case 1200:
        dmc->T_RDDATA_EN_NEXT = 0x000E000E;
        break;
    case 1333:
        dmc->T_RDDATA_EN_NEXT = 0x000E000E;
        break;
    default:
        fwk_assert(false);
        break;
    }

    dmc->T_PHYRDLAT_NEXT = 0x00000046;

    switch (ddr_info.speed) {
    case 800:
        dmc->T_PHYWRLAT_NEXT = 0x01050009;
        break;
    case 1200:
        dmc->T_PHYWRLAT_NEXT = 0x0105000C;
        break;
    case 1333:
        dmc->T_PHYWRLAT_NEXT = 0x010A000E;
        break;
    default:
        fwk_assert(false);
        break;
    }

    dmc->RDLVL_CONTROL_NEXT = 0x01000000;
    dmc->RDLVL_MRS_NEXT = 0x00000224;
    dmc->T_RDLVL_RR_NEXT = 0x0000003E;
    dmc->WRLVL_CONTROL_NEXT = 0x00100000;
    dmc->WRLVL_MRS_NEXT = 0x00000083;
    dmc->T_WRLVL_EN_NEXT = 0x0000003E;
    dmc->PHY_UPDATE_CONTROL_NEXT = 0x01401111;
    dmc->T_LVL_DISCONNECT_NEXT = 0x00000001;
    dmc->WDQLVL_CONTROL_NEXT = 0x00000080;
    dmc->T_WDQLVL_EN_NEXT = 0x00000024;
    dmc->T_WDQLVL_WW_NEXT = 0x00000006;
    dmc->T_WDQLVL_RW_NEXT = 0x00000009;
    dmc->ERR0CTLR1 = 0x000000D1;
    dmc->RANK_REMAP_CONTROL_NEXT = 0x76543210;
    dmc->PHY_REQUEST_CS_REMAP = 0x76543210;
    dmc->T_ODTH_NEXT = 0x00000006;
    dmc->ODT_TIMING_NEXT = 0x06000600;
    dmc->T_RW_ODT_CLR_NEXT = 0x0000000f;
    dmc->T_CMD_NEXT = 0x00000000;
    dmc->T_RDLVL_EN_NEXT = 0x0000003E;
    dmc->T_WRLVL_WW_NEXT = 0x0000003E;
    dmc->PHYMSTR_CONTROL_NEXT = 0x00000000;
    dmc->T_LPRESP_NEXT = 0x00000005;
    dmc->ODT_WR_CONTROL_31_00_NEXT = 0x08040201;
    dmc->ODT_WR_CONTROL_63_32_NEXT = 0x08040201;
    dmc->ODT_RD_CONTROL_31_00_NEXT = 0x00000000;
    dmc->ODT_RD_CONTROL_63_32_NEXT = 0x00000000;
    dmc->ODT_CP_CONTROL_31_00_NEXT = 0x08040201;
    dmc->ODT_CP_CONTROL_63_32_NEXT = 0x80402010;
    dmc->CS_REMAP_CONTROL_31_00_NEXT = 0x00020001;
    dmc->CS_REMAP_CONTROL_63_32_NEXT = 0x00080004;
    dmc->CS_REMAP_CONTROL_95_64_NEXT = 0x00200010;
    dmc->CS_REMAP_CONTROL_127_96_NEXT = 0x00800040;
    dmc->CID_REMAP_CONTROL_31_00_NEXT = 0x00000000;
    dmc->CID_REMAP_CONTROL_63_32_NEXT = 0x00000000;
    dmc->POWER_GROUP_CONTROL_31_00_NEXT = 0x00020001;
    dmc->POWER_GROUP_CONTROL_63_32_NEXT = 0x00080004;
    dmc->POWER_GROUP_CONTROL_95_64_NEXT = 0x00200010;
    dmc->POWER_GROUP_CONTROL_127_96_NEXT = 0x00800040;
    dmc->REFRESH_CONTROL_NEXT = 0x00000000;
    dmc->T_DB_TRAIN_RESP_NEXT = 0x00000004;
    dmc->FEATURE_CONTROL_NEXT = 0x0aa30000;
    dmc->MUX_CONTROL_NEXT = 0x00000000;
    dmc->LOW_POWER_CONTROL_NEXT = 0x00000010;
    dmc->MEMORY_ADDRESS_MAX_31_00_NEXT = 0xffff001f;
    dmc->INTERRUPT_CONTROL = 0x00000070;

    dmc->DIRECT_CMD = (ddr_info.ranks_to_train << 16) | 0x000C;

    dmc->USER_CONFIG0_NEXT = 0x1;
    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_GO;
    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) != MOD_DMC620_MEMC_CMD_GO)
        continue;

    status = ddr_phy_api->configure(ddr_id, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_CONFIG;
    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) !=
           MOD_DMC620_MEMC_CMD_CONFIG)
        continue;
    dmc->USER_CONFIG0_NEXT = 0x3;
    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_GO;
    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) != MOD_DMC620_MEMC_CMD_GO)
        continue;
    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_CONFIG;
    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) !=
           MOD_DMC620_MEMC_CMD_CONFIG)
        continue;

    FWK_LOG_INFO("[DDR] Sending direct DDR commands");

    status = direct_ddr_cmd(dmc);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_training(dmc, ddr_id, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_phy_api->post_training_configure(ddr_id, &ddr_info);
    if (status != FWK_SUCCESS)
        return status;

    FWK_LOG_INFO("[DDR] Enable DIMM refresh...");
    status = enable_dimm_refresh(dmc);
    if (status != FWK_SUCCESS)
        return status;

    /* Switch to READY */
    FWK_LOG_INFO("[DDR] Setting DMC to READY mode");

    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_GO;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) != MOD_DMC620_MEMC_CMD_GO)
        continue;

    FWK_LOG_INFO("[DDR] DMC init done.");

    if (dmc_id == 1) {
        status = dmc620_post_init();
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

/* Memory Information API */

static int dmc620_get_mem_size_gb(uint32_t *size)
{
    uint32_t size_gb;
    int status;

    fwk_assert(size != NULL);

    size_gb = 0;
    status = dimm_spd_calculate_dimm_size_gb(&size_gb);
    if (status != FWK_SUCCESS)
        return status;

    *size = size_gb * 2;
    return FWK_SUCCESS;
}

struct mod_dmc620_mem_info_api ddr_mem_info_api = {
    .get_mem_size_gb = dmc620_get_mem_size_gb,
};

/* Framework API */
static int mod_dmc620_init(fwk_id_t module_id, unsigned int element_count,
                           const void *config)
{
    fwk_assert(config != NULL);
    struct mod_dmc620_module_config *mod_config =
        (struct mod_dmc620_module_config *)config;

    ddr_info.speed = mod_config->ddr_speed;
    return FWK_SUCCESS;
}

static int mod_dmc620_element_init(fwk_id_t element_id, unsigned int unused,
                                   const void *data)
{
    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int mod_dmc620_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_dmc620_module_config *module_config;

    /* Nothing to do in the second round of calls. */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to do in case of elements. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    module_config = fwk_module_get_data(fwk_module_id_n1sdp_dmc620);
    fwk_assert(module_config != NULL);

    status = fwk_module_bind(module_config->ddr_module_id,
                             module_config->ddr_api_id, &ddr_phy_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
        &timer_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_I2C),
                             FWK_ID_API(FWK_MODULE_IDX_N1SDP_I2C,
                                        MOD_N1SDP_I2C_API_MASTER_POLLED),
                             &i2c_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int mod_dmc620_process_bind_request(fwk_id_t requester_id,
    fwk_id_t id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_DMC620_API_IDX_MEM_INFO:
        *api = &ddr_mem_info_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int mod_dmc620_start(fwk_id_t id)
{
    const struct mod_dmc620_element_config *element_config;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* Bypass divide by 2 so DMCCLK1X = DMCCLK2X */
        PIK_SYSTEM->DMCCLK_CTRL |= PIK_SYSTEM_DMCCLK_CTRL_DIV2_BYPASS_MASK;
        return FWK_SUCCESS;
    }

    element_config = fwk_module_get_data(id);

    /* Register elements for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        element_config->clock_id,
        id);
}

static int dmc620_notify_system_state_transition_resume(fwk_id_t id)
{
    struct mod_dmc620_reg *dmc;
    const struct mod_dmc620_element_config *element_config;

    element_config = fwk_module_get_data(id);
    dmc = (struct mod_dmc620_reg *)element_config->dmc;

    return dmc620_config(dmc, element_config->ddr_id);
}

static int mod_dmc620_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    fwk_assert(fwk_id_is_equal(event->id,
                               mod_clock_notification_id_state_changed));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING)
        return dmc620_notify_system_state_transition_resume(event->target_id);

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_dmc620 = {
    .name = "N1SDP-DMC620",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_dmc620_init,
    .element_init = mod_dmc620_element_init,
    .bind = mod_dmc620_bind,
    .start = mod_dmc620_start,
    .process_notification = mod_dmc620_process_notification,
    .process_bind_request = mod_dmc620_process_bind_request,
    .api_count = MOD_DMC620_API_COUNT,
    .event_count = 0,
};
