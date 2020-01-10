/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DMC-620 driver
 */

#include <mod_clock.h>
#include <mod_dmc620.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stddef.h>

static struct mod_dmc_ddr_phy_api *ddr_phy_api;

static int dmc620_config(struct mod_dmc620_reg *dmc, fwk_id_t ddr_id);

/* Framework API */
static int mod_dmc620_init(fwk_id_t module_id, unsigned int element_count,
                           const void *config)
{
    return FWK_SUCCESS;
}

static int mod_dmc620_element_init(fwk_id_t element_id, unsigned int unused,
                                   const void *data)
{
    assert(data != NULL);

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

    module_config = fwk_module_get_data(fwk_module_id_dmc620);
    assert(module_config != NULL);

    status = fwk_module_bind(module_config->ddr_module_id,
                             module_config->ddr_api_id, &ddr_phy_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int mod_dmc620_start(fwk_id_t id)
{
    const struct mod_dmc620_element_config *element_config;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

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

    assert(fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING)
        return dmc620_notify_system_state_transition_resume(event->target_id);

    return FWK_SUCCESS;
}

const struct fwk_module module_dmc620 = {
    .name = "DMC620",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_dmc620_init,
    .element_init = mod_dmc620_element_init,
    .bind = mod_dmc620_bind,
    .start = mod_dmc620_start,
    .process_notification = mod_dmc620_process_notification,
    .api_count = 0,
    .event_count = 0,
};

static int dmc620_config(struct mod_dmc620_reg *dmc, fwk_id_t ddr_id)
{
    int i;
    struct mod_dmc620_reg *reg_val;
    const struct mod_dmc620_module_config *module_config;

    module_config = fwk_module_get_data(fwk_module_id_dmc620);
    reg_val = module_config->dmc_val;

    FWK_LOG_INFO("[DDR] Initialising DMC620 at 0x%x", (uintptr_t)dmc);

    FWK_LOG_INFO("[DDR] Writing functional settings");

    dmc->ADDRESS_CONTROL_NEXT = reg_val->ADDRESS_CONTROL_NEXT;

    dmc->DECODE_CONTROL_NEXT = reg_val->DECODE_CONTROL_NEXT;
    dmc->FORMAT_CONTROL = reg_val->FORMAT_CONTROL;

    dmc->ADDRESS_MAP_NEXT = reg_val->ADDRESS_MAP_NEXT;

    dmc->LOW_POWER_CONTROL_NEXT = reg_val->LOW_POWER_CONTROL_NEXT;
    dmc->TURNAROUND_CONTROL_NEXT = reg_val->TURNAROUND_CONTROL_NEXT;
    dmc->HIT_TURNAROUND_CONTROL_NEXT = reg_val->HIT_TURNAROUND_CONTROL_NEXT;
    dmc->QOS_CLASS_CONTROL_NEXT = reg_val->QOS_CLASS_CONTROL_NEXT;
    dmc->ESCALATION_CONTROL_NEXT = reg_val->ESCALATION_CONTROL_NEXT;
    dmc->QV_CONTROL_31_00_NEXT = reg_val->QV_CONTROL_31_00_NEXT;
    dmc->QV_CONTROL_63_32_NEXT = reg_val->QV_CONTROL_63_32_NEXT;
    dmc->RT_CONTROL_31_00_NEXT = reg_val->RT_CONTROL_31_00_NEXT;
    dmc->RT_CONTROL_63_32_NEXT = reg_val->RT_CONTROL_63_32_NEXT;
    dmc->TIMEOUT_CONTROL_NEXT = reg_val->TIMEOUT_CONTROL_NEXT;
    dmc->CREDIT_CONTROL_NEXT = reg_val->CREDIT_CONTROL_NEXT;
    dmc->WRITE_PRIORITY_CONTROL_31_00_NEXT =
        reg_val->WRITE_PRIORITY_CONTROL_31_00_NEXT;
    dmc->WRITE_PRIORITY_CONTROL_63_32_NEXT =
        reg_val->WRITE_PRIORITY_CONTROL_63_32_NEXT;
    dmc->QUEUE_THRESHOLD_CONTROL_31_00_NEXT =
        reg_val->QUEUE_THRESHOLD_CONTROL_31_00_NEXT;
    dmc->QUEUE_THRESHOLD_CONTROL_63_32_NEXT =
        reg_val->QUEUE_THRESHOLD_CONTROL_63_32_NEXT;
    dmc->ADDRESS_SHUTTER_31_00_NEXT = reg_val->ADDRESS_SHUTTER_31_00_NEXT;
    dmc->ADDRESS_SHUTTER_63_32_NEXT = reg_val->ADDRESS_SHUTTER_63_32_NEXT;
    dmc->ADDRESS_SHUTTER_95_64_NEXT = reg_val->ADDRESS_SHUTTER_95_64_NEXT;
    dmc->ADDRESS_SHUTTER_127_96_NEXT = reg_val->ADDRESS_SHUTTER_127_96_NEXT;
    dmc->ADDRESS_SHUTTER_159_128_NEXT = reg_val->ADDRESS_SHUTTER_159_128_NEXT;
    dmc->ADDRESS_SHUTTER_191_160_NEXT = reg_val->ADDRESS_SHUTTER_191_160_NEXT;
    dmc->MEMORY_ADDRESS_MAX_31_00_NEXT = reg_val->MEMORY_ADDRESS_MAX_31_00_NEXT;
    dmc->MEMORY_ADDRESS_MAX_43_32_NEXT = reg_val->MEMORY_ADDRESS_MAX_43_32_NEXT;
    dmc->ACCESS_ADDRESS_NEXT[0].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[0].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[0].MIN_43_32 =
        reg_val->ACCESS_ADDRESS_NEXT[0].MIN_43_32;
    dmc->ACCESS_ADDRESS_NEXT[1].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[1].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[1].MIN_43_32 =
        reg_val->ACCESS_ADDRESS_NEXT[1].MIN_43_32;
    dmc->ACCESS_ADDRESS_NEXT[2].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[2].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[3].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[3].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[4].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[4].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[5].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[5].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[6].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[6].MIN_31_00;
    dmc->ACCESS_ADDRESS_NEXT[7].MIN_31_00 =
        reg_val->ACCESS_ADDRESS_NEXT[7].MIN_31_00;

    dmc->DCI_REPLAY_TYPE_NEXT = reg_val->DCI_REPLAY_TYPE_NEXT;
    dmc->DCI_STRB = reg_val->DCI_STRB;
    dmc->DCI_DATA = reg_val->DCI_DATA;
    dmc->REFRESH_CONTROL_NEXT = reg_val->REFRESH_CONTROL_NEXT;
    dmc->MEMORY_TYPE_NEXT = reg_val->MEMORY_TYPE_NEXT;
    dmc->FEATURE_CONFIG = reg_val->FEATURE_CONFIG;
    dmc->FEATURE_CONTROL_NEXT = reg_val->FEATURE_CONTROL_NEXT;
    dmc->MUX_CONTROL_NEXT = reg_val->MUX_CONTROL_NEXT;

    /* Timing Configuration */
    FWK_LOG_INFO("[DDR] Writing timing settings");

    dmc->T_REFI_NEXT = reg_val->T_REFI_NEXT;
    dmc->T_RFC_NEXT = reg_val->T_RFC_NEXT;
    dmc->T_MRR_NEXT = reg_val->T_MRR_NEXT;
    dmc->T_MRW_NEXT = reg_val->T_MRW_NEXT;
    dmc->T_RCD_NEXT = reg_val->T_RCD_NEXT;
    dmc->T_RAS_NEXT = reg_val->T_RAS_NEXT;
    dmc->T_RP_NEXT = reg_val->T_RP_NEXT;
    dmc->T_RPALL_NEXT = reg_val->T_RPALL_NEXT;
    dmc->T_RRD_NEXT = reg_val->T_RRD_NEXT;
    dmc->T_ACT_WINDOW_NEXT = reg_val->T_ACT_WINDOW_NEXT;
    dmc->T_RTR_NEXT = reg_val->T_RTR_NEXT;
    dmc->T_RTW_NEXT = reg_val->T_RTW_NEXT;
    dmc->T_RTP_NEXT = reg_val->T_RTP_NEXT;
    dmc->T_WR_NEXT = reg_val->T_WR_NEXT;
    dmc->T_WTR_NEXT = reg_val->T_WTR_NEXT;
    dmc->T_WTW_NEXT = reg_val->T_WTW_NEXT;
    dmc->T_XMPD_NEXT = reg_val->T_XMPD_NEXT;
    dmc->T_EP_NEXT = reg_val->T_EP_NEXT;
    dmc->T_XP_NEXT = reg_val->T_XP_NEXT;
    dmc->T_ESR_NEXT = reg_val->T_ESR_NEXT;
    dmc->T_XSR_NEXT = reg_val->T_XSR_NEXT;
    dmc->T_ESRCK_NEXT = reg_val->T_ESRCK_NEXT;
    dmc->T_CKXSR_NEXT = reg_val->T_CKXSR_NEXT;
    dmc->T_CMD_NEXT = reg_val->T_CMD_NEXT;
    dmc->T_PARITY_NEXT = reg_val->T_PARITY_NEXT;
    dmc->T_ZQCS_NEXT = reg_val->T_ZQCS_NEXT;
    dmc->T_RW_ODT_CLR_NEXT = reg_val->T_RW_ODT_CLR_NEXT;
    dmc->T_RDDATA_EN_NEXT = reg_val->T_RDDATA_EN_NEXT;
    dmc->T_PHYWRLAT_NEXT = reg_val->T_PHYWRLAT_NEXT;
    dmc->T_PHYRDLAT_NEXT = reg_val->T_PHYRDLAT_NEXT;
    dmc->RDLVL_CONTROL_NEXT = reg_val->RDLVL_CONTROL_NEXT;
    dmc->RDLVL_MRS_NEXT = reg_val->RDLVL_MRS_NEXT;
    dmc->T_RDLVL_EN_NEXT = reg_val->T_RDLVL_EN_NEXT;
    dmc->T_RDLVL_RR_NEXT = reg_val->T_RDLVL_RR_NEXT;
    dmc->WRLVL_CONTROL_NEXT = reg_val->WRLVL_CONTROL_NEXT;
    dmc->WRLVL_MRS_NEXT = reg_val->WRLVL_MRS_NEXT;
    dmc->T_WRLVL_EN_NEXT = reg_val->T_WRLVL_EN_NEXT;
    dmc->T_WRLVL_WW_NEXT = reg_val->T_WRLVL_WW_NEXT;
    dmc->PHY_POWER_CONTROL_NEXT = reg_val->PHY_POWER_CONTROL_NEXT;
    dmc->T_LPRESP_NEXT = reg_val->T_LPRESP_NEXT;
    dmc->PHY_UPDATE_CONTROL_NEXT = reg_val->PHY_UPDATE_CONTROL_NEXT;
    dmc->T_ODTH_NEXT = reg_val->T_ODTH_NEXT;

    dmc->ODT_TIMING_NEXT = reg_val->ODT_TIMING_NEXT;
    dmc->ODT_WR_CONTROL_31_00_NEXT = reg_val->ODT_WR_CONTROL_31_00_NEXT;
    dmc->ODT_WR_CONTROL_63_32_NEXT = reg_val->ODT_WR_CONTROL_63_32_NEXT;
    dmc->ODT_RD_CONTROL_31_00_NEXT = reg_val->ODT_RD_CONTROL_31_00_NEXT;
    dmc->ODT_RD_CONTROL_63_32_NEXT = reg_val->ODT_RD_CONTROL_63_32_NEXT;

    /* Enable RAS interrupts and error detection */
    dmc->ERR0CTLR0 = reg_val->ERR0CTLR0;

    ddr_phy_api->configure(ddr_id);

    for (i = 0; i < 3; i++) /* ~200ns */
        __NOP();

    FWK_LOG_INFO("[DDR] Sending direct DDR commands");

    module_config->direct_ddr_cmd(dmc);

    for (i = 0; i < 5; i++) /* ~400ns */
        __NOP();

    /* Switch to READY */
    FWK_LOG_INFO("[DDR] Setting DMC to READY mode");

    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_GO;
    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_EXECUTE;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) != MOD_DMC620_MEMC_CMD_GO)
        continue;

    FWK_LOG_INFO("[DDR] DMC init done.");

    return FWK_SUCCESS;
}
