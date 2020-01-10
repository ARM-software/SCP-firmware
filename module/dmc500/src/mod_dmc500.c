/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DMC-500 driver
 */

#include <mod_dmc500.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

static struct mod_dmc_ddr_phy_api *ddr_phy_api;
static struct mod_timer_api *timer_api;

static int dmc500_config(struct mod_dmc500_reg *dmc, fwk_id_t ddr_phy_id);

/* Framework API */
static int mod_dmc500_init(fwk_id_t module_id, unsigned int element_count,
                           const void *data)
{
    return FWK_SUCCESS;
}

static int mod_dmc500_element_init(fwk_id_t element_id, unsigned int unused,
                                   const void *data)
{
    assert(data != NULL);

    return FWK_SUCCESS;
}

static int mod_dmc500_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_dmc500_module_config *module_config;

    /* Nothing to do in the second round of calls. */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to do in case of elements. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    module_config = fwk_module_get_data(fwk_module_id_dmc500);
    assert(module_config != NULL);

    status = fwk_module_bind(module_config->ddr_phy_module_id,
                             module_config->ddr_phy_api_id, &ddr_phy_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(module_config->timer_id,
                             FWK_ID_API(FWK_MODULE_IDX_TIMER, 0), &timer_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int mod_dmc500_start(fwk_id_t id)
{
    const struct mod_dmc500_element_config *element_config;
    struct mod_dmc500_reg *dmc;

    /* Nothing to start for the module */
    if (fwk_module_is_valid_module_id(id))
        return FWK_SUCCESS;

    element_config = fwk_module_get_data(id);
    dmc = (struct mod_dmc500_reg *)element_config->dmc;

    return dmc500_config(dmc, element_config->ddr_phy_id);
}

const struct fwk_module module_dmc500 = {
    .name = "DMC500",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_dmc500_init,
    .element_init = mod_dmc500_element_init,
    .bind = mod_dmc500_bind,
    .start = mod_dmc500_start,
    .api_count = 0,
    .event_count = 0,
};


static int dmc500_config(struct mod_dmc500_reg *dmc, fwk_id_t ddr_phy_id)
{
    int status;
    uint64_t timeout;
    uint64_t remaining_ticks;
    uint64_t counter;
    const struct mod_dmc500_reg *reg_val;
    const struct mod_dmc500_module_config *module_config;

    module_config = fwk_module_get_data(fwk_module_id_dmc500);
    reg_val = module_config->reg_val;

    FWK_LOG_INFO("[DDR] Initialising DMC500 at 0x%x", (uintptr_t)dmc);

    dmc->ADDRESS_CONTROL      = reg_val->ADDRESS_CONTROL;
    dmc->RANK_REMAP_CONTROL   = reg_val->RANK_REMAP_CONTROL;
    dmc->MEMORY_TYPE          = reg_val->MEMORY_TYPE;
    dmc->FORMAT_CONTROL       = reg_val->FORMAT_CONTROL;
    dmc->DECODE_CONTROL       = reg_val->DECODE_CONTROL;
    dmc->FEATURE_CONTROL      = reg_val->FEATURE_CONTROL;
    dmc->ODT_WR_CONTROL_31_00 = reg_val->ODT_WR_CONTROL_31_00;
    dmc->ODT_RD_CONTROL_31_00 = reg_val->ODT_RD_CONTROL_31_00;
    dmc->ODT_TIMING           = reg_val->ODT_TIMING;

    FWK_LOG_INFO("[DDR] Setting timing settings");

    dmc->T_REFI          = reg_val->T_REFI;
    dmc->T_RFC           = reg_val->T_RFC;
    dmc->T_RDPDEN        = reg_val->T_RDPDEN;
    dmc->T_RCD           = reg_val->T_RCD;
    dmc->T_RAS           = reg_val->T_RAS;
    dmc->T_RP            = reg_val->T_RP;
    dmc->T_RRD           = reg_val->T_RRD;
    dmc->T_ACT_WINDOW    = reg_val->T_ACT_WINDOW;
    dmc->T_RTR           = reg_val->T_RTR;
    dmc->T_RTW           = reg_val->T_RTW;
    dmc->T_RTP           = reg_val->T_RTP;
    dmc->T_WR            = reg_val->T_WR;
    dmc->T_WTR           = reg_val->T_WTR;
    dmc->T_WTW           = reg_val->T_WTW;
    dmc->T_XTMW          = reg_val->T_XTMW;
    dmc->T_CLOCK_CONTROL = reg_val->T_CLOCK_CONTROL;
    dmc->T_EP            = reg_val->T_EP;
    dmc->T_XP            = reg_val->T_XP;
    dmc->T_ESR           = reg_val->T_ESR;
    dmc->T_XSR           = reg_val->T_XSR;

    FWK_LOG_INFO("[DDR] Setting address map");

    dmc->ADDRESS_MAP = reg_val->ADDRESS_MAP;

    FWK_LOG_INFO("[DDR] Setting PMU settings");

    dmc->SI0_SI_INTERRUPT_CONTROL = reg_val->SI0_SI_INTERRUPT_CONTROL;
    dmc->SI0_PMU_REQ_CONTROL = reg_val->SI0_PMU_REQ_CONTROL;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_0 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MASK_0;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_0 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MATCH_0;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_1 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MASK_1;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_1 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MATCH_1;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_2 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MASK_2;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_2 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MATCH_2;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_3 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MASK_3;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_3 = reg_val->SI0_PMU_REQ_ATTRIBUTE_MATCH_3;
    dmc->SI0_THRESHOLD_CONTROL = reg_val->SI0_THRESHOLD_CONTROL;
    dmc->SI1_SI_INTERRUPT_CONTROL = reg_val->SI1_SI_INTERRUPT_CONTROL;
    dmc->SI1_PMU_REQ_CONTROL = reg_val->SI1_PMU_REQ_CONTROL;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_0 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MASK_0;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_0 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MATCH_0;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_1 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MASK_1;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_1 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MATCH_1;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_2 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MASK_2;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_2 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MATCH_2;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_3 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MASK_3;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_3 = reg_val->SI1_PMU_REQ_ATTRIBUTE_MATCH_3;
    dmc->SI1_THRESHOLD_CONTROL = reg_val->SI1_THRESHOLD_CONTROL;
    dmc->QUEUE_THRESHOLD_CONTROL_31_00 = reg_val->QUEUE_THRESHOLD_CONTROL_31_00;
    dmc->QUEUE_THRESHOLD_CONTROL_63_32 = reg_val->QUEUE_THRESHOLD_CONTROL_63_32;
    dmc->DCB_INTERRUPT_CONTROL = reg_val->DCB_INTERRUPT_CONTROL;
    dmc->PMU_DCB_CONTROL = reg_val->PMU_DCB_CONTROL;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_0 =
        reg_val->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_0;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_0 =
        reg_val->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_0;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_1 =
        reg_val->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_1;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_1 =
        reg_val->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_1;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_2 =
        reg_val->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_2;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_2 =
        reg_val->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_2;
    dmc->PMU_TAG_ENTRIES_ATTRIBUTE_MASK =
        reg_val->PMU_TAG_ENTRIES_ATTRIBUTE_MASK;
    dmc->PMU_TAG_ENTRIES_ATTRIBUTE_MATCH =
        reg_val->PMU_TAG_ENTRIES_ATTRIBUTE_MATCH;
    dmc->QE_INTERRUPT_CONTROL = reg_val->QE_INTERRUPT_CONTROL;
    dmc->RANK_TURNAROUND_CONTROL = reg_val->RANK_TURNAROUND_CONTROL;
    dmc->HIT_TURNAROUND_CONTROL = reg_val->HIT_TURNAROUND_CONTROL;
    dmc->QOS_CLASS_CONTROL = reg_val->QOS_CLASS_CONTROL;
    dmc->ESCALATION_CONTROL = reg_val->ESCALATION_CONTROL;
    dmc->QV_CONTROL_31_00 = reg_val->QV_CONTROL_31_00;
    dmc->QV_CONTROL_63_32 = reg_val->QV_CONTROL_63_32;
    dmc->RT_CONTROL_31_00 = reg_val->RT_CONTROL_31_00;
    dmc->RT_CONTROL_63_32 = reg_val->RT_CONTROL_63_32;
    dmc->TIMEOUT_CONTROL = reg_val->TIMEOUT_CONTROL;
    dmc->WRITE_PRIORITY_CONTROL_31_00 = reg_val->WRITE_PRIORITY_CONTROL_31_00;
    dmc->WRITE_PRIORITY_CONTROL_63_32 = reg_val->WRITE_PRIORITY_CONTROL_63_32;
    dmc->DIR_TURNAROUND_CONTROL = reg_val->DIR_TURNAROUND_CONTROL;
    dmc->HIT_PREDICTION_CONTROL = reg_val->HIT_PREDICTION_CONTROL;
    dmc->REFRESH_PRIORITY = reg_val->REFRESH_PRIORITY;
    dmc->MC_UPDATE_CONTROL = reg_val->MC_UPDATE_CONTROL;
    dmc->PHY_UPDATE_CONTROL = reg_val->PHY_UPDATE_CONTROL;
    dmc->PHY_MASTER_CONTROL = reg_val->PHY_MASTER_CONTROL;
    dmc->LOW_POWER_CONTROL = reg_val->LOW_POWER_CONTROL;
    dmc->PMU_QE_CONTROL = reg_val->PMU_QE_CONTROL;
    dmc->PMU_QE_MUX = reg_val->PMU_QE_MUX;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MASK_0 =
        reg_val->PMU_QOS_ENGINE_ATTRIBUTE_MASK_0;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_0 =
        reg_val->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_0;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MASK_1 =
        reg_val->PMU_QOS_ENGINE_ATTRIBUTE_MASK_1;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_1 =
        reg_val->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_1;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MASK_2 =
        reg_val->PMU_QOS_ENGINE_ATTRIBUTE_MASK_2;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_2 =
        reg_val->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_2;
    dmc->PMU_QUEUED_ENTRIES_ATTRIBUTE_MASK =
        reg_val->PMU_QUEUED_ENTRIES_ATTRIBUTE_MASK;
    dmc->PMU_QUEUED_ENTRIES_ATTRIBUTE_MATCH =
        reg_val->PMU_QUEUED_ENTRIES_ATTRIBUTE_MATCH;
    dmc->MI_INTERRUPT_CONTROL = reg_val->MI_INTERRUPT_CONTROL;
    dmc->POWER_DOWN_CONTROL = reg_val->POWER_DOWN_CONTROL;
    dmc->REFRESH_CONTROL = reg_val->REFRESH_CONTROL;
    dmc->PMU_MI_CONTROL = reg_val->PMU_MI_CONTROL;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MASK_0 =
        reg_val->PMU_MEMORY_IF_ATTRIBUTE_MASK_0;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MATCH_0 =
        reg_val->PMU_MEMORY_IF_ATTRIBUTE_MATCH_0;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MASK_1 =
        reg_val->PMU_MEMORY_IF_ATTRIBUTE_MASK_1;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MATCH_1 =
        reg_val->PMU_MEMORY_IF_ATTRIBUTE_MATCH_1;
    dmc->PMU_BANK_STATES_ATTRIBUTE_MASK =
        reg_val->PMU_BANK_STATES_ATTRIBUTE_MASK;
    dmc->PMU_BANK_STATES_ATTRIBUTE_MATCH =
        reg_val->PMU_BANK_STATES_ATTRIBUTE_MATCH;
    dmc->PMU_RANK_STATES_ATTRIBUTE_MASK =
        reg_val->PMU_RANK_STATES_ATTRIBUTE_MASK;
    dmc->PMU_RANK_STATES_ATTRIBUTE_MATCH =
        reg_val->PMU_RANK_STATES_ATTRIBUTE_MATCH;
    dmc->CFG_INTERRUPT_CONTROL = reg_val->CFG_INTERRUPT_CONTROL;
    dmc->T_RDDATA_EN = reg_val->T_RDDATA_EN;
    dmc->T_PHYRDLAT = reg_val->T_PHYRDLAT;
    dmc->T_PHYWRLAT = reg_val->T_PHYWRLAT;

    dmc->ERR_RAMECC_CTLR = reg_val->ERR_RAMECC_CTLR;

    FWK_LOG_INFO("[DDR] Setting PHY-related settings");

    dmc->PHY_POWER_CONTROL = reg_val->PHY_POWER_CONTROL;
    dmc->T_PHY_TRAIN = reg_val->T_PHY_TRAIN;
    dmc->PHYUPD_INIT = reg_val->PHYUPD_INIT;

    dmc->PHY_CONFIG = 0x03000000;
    dmc->PHY_CONFIG = 0x0600000A;
    dmc->PHY_CONFIG = 0x01000001;

    status = ddr_phy_api->configure(ddr_phy_id);
    if (status != FWK_SUCCESS)
        return status;

    dmc->PHY_CONFIG = 0x01000001;
    dmc->PHY_CONFIG = 0x01000000;
    dmc->PHY_CONFIG = 0x00000003;

    FWK_LOG_INFO("[DDR] Doing direct DDR commands");

    module_config->direct_ddr_cmd(dmc);

    dmc->REFRESH_ENABLE = reg_val->REFRESH_ENABLE;

    FWK_LOG_INFO("[DDR] Setting dmc in READY mode");

    status = timer_api->time_to_timestamp(module_config->timer_id,
                                          1000 * 1000, &timeout);
    if (status != FWK_SUCCESS)
        return status;

    status = timer_api->get_counter(module_config->timer_id, &counter);
    if (status != FWK_SUCCESS)
        return status;

    timeout += counter;

    while ((dmc->MI_STATUS & MOD_DMC500_MI_STATUS_IDLE) !=
        MOD_DMC500_MI_STATUS_IDLE) {
        status = timer_api->remaining(module_config->timer_id, timeout,
                                      &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;

        if (remaining_ticks == 0)
            goto timeout;
    }

    dmc->MI_STATE_CONTROL     = reg_val->MI_STATE_CONTROL;
    dmc->QUEUE_STATE_CONTROL  = reg_val->QUEUE_STATE_CONTROL;
    dmc->SI0_SI_STATE_CONTROL = reg_val->SI0_SI_STATE_CONTROL;
    dmc->SI1_SI_STATE_CONTROL = reg_val->SI1_SI_STATE_CONTROL;

    FWK_LOG_INFO("[DDR] Waiting for Queue stall = 0...");

    while ((dmc->QUEUE_STATUS & MOD_DMC500_QUEUE_STATUS_STALL_ACK) != 0) {
        status = timer_api->remaining(module_config->timer_id, timeout,
                                      &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;

        if (remaining_ticks == 0)
            goto timeout;
    }

    FWK_LOG_INFO("[DDR] Waiting for SI0 stall = 0...");

    while ((dmc->SI0_SI_STATUS & MOD_DMC500_SI_STATUS_STALL_ACK) != 0) {
        status = timer_api->remaining(module_config->timer_id, timeout,
                                      &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;

        if (remaining_ticks == 0)
            goto timeout;
    }

    FWK_LOG_INFO("[DDR] Waiting for SI1 stall = 0...");

    while ((dmc->SI1_SI_STATUS & MOD_DMC500_SI_STATUS_STALL_ACK) != 0) {
        status = timer_api->remaining(module_config->timer_id, timeout,
                                      &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;

        if (remaining_ticks == 0)
            goto timeout;
    }

    FWK_LOG_INFO("[DDR] DMC init done.");

    return FWK_SUCCESS;

timeout:
    FWK_LOG_ERR("[DDR] Timed out in DMC500 init.");

    return FWK_E_TIMEOUT;
}
