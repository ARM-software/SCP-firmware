/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP DMC-620 driver
 */

#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_interrupt.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_log.h>
#include <mod_n1sdp_dmc620.h>
#include <mod_power_domain.h>
#include <mod_timer.h>
#include <config_clock.h>
#include <n1sdp_scp_irq.h>
#include <n1sdp_scp_pik.h>
#include <n1sdp_pik_system.h>

/* DMC-620 register specific definitions */
#define DDR_TRAIN_TWO_RANKS          0

static struct mod_log_api *log_api;
static struct mod_dmc_ddr_phy_api *ddr_phy_api;
static struct mod_timer_api *timer_api;

static int dmc620_config(struct mod_dmc620_reg *dmc, fwk_id_t ddr_id);
static void direct_ddr_cmd(struct mod_dmc620_reg *dmc);
static int enable_dimm_refresh(struct mod_dmc620_reg *dmc);
static int dmc620_config_interrupt(fwk_id_t ddr_id);

/* Memory Information API */

static void dmc620_get_mem_size_gb(uint32_t *size)
{
    fwk_assert(size != NULL);
    *size = 16;
}

struct mod_dmc620_mem_info_api ddr_mem_info_api = {
    .get_mem_size_gb = dmc620_get_mem_size_gb,
};

/* Framework API */
static int mod_dmc620_init(fwk_id_t module_id, unsigned int element_count,
                           const void *config)
{
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

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
                             MOD_LOG_API_ID, &log_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(module_config->ddr_module_id,
                             module_config->ddr_api_id, &ddr_phy_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
        &timer_api);
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
        log_api->log(MOD_LOG_GROUP_INFO, "FAIL\n");
        return status;
    }

    wait_data.stage = DMC620_CONFIG_STAGE_TRAINING_M0_IDLE;
    status = timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                             DMC_TRAINING_TIMEOUT,
                             dmc620_wait_condition,
                             &wait_data);
    if (status != FWK_SUCCESS) {
        log_api->log(MOD_LOG_GROUP_INFO, "FAIL\n");
        return status;
    }

    log_api->log(MOD_LOG_GROUP_INFO, "PASS\n");

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

static int ddr_training(struct mod_dmc620_reg *dmc)
{
    uint32_t value;
    int status;

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Training DDR memories...\n");

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Write leveling... ");

    /* Set training command */
    dmc->DIRECT_ADDR = DDR_ADDR_TRAIN_TYPE_WR_LVL;
    dmc->DIRECT_CMD  = DDR_CMD_TRAIN_RANK_1;

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Read gate training\n");

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] A side...");

    /* Set write leveling parameters */
    value = dmc->RDLVL_CONTROL_NEXT;
    value |= (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;
    /* Update */
    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = 0x0001000C;

    /* Run training on slices 0-9 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_GATE |
                        (0x3FFFF << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = DDR_CMD_TRAIN_RANK_1;

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

#if DDR_TRAIN_TWO_RANKS
    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] B side...");

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

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Read eye training\n");

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] A side...");

    /* Set write leveling parameters */
    value = dmc->RDLVL_CONTROL_NEXT;
    value |= (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;
    /* Update */
    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = 0x0001000C;

    /* Run training on slices 0-9 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_EYE |
                        (0x201FF << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = DDR_CMD_TRAIN_RANK_1;

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] B side...");

    /* Set write leveling parameters */
    value = dmc->RDLVL_CONTROL_NEXT;
    value |= (0x03 << 9) | (0 << 16);
    dmc->RDLVL_CONTROL_NEXT = value;
    /* Update */
    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = 0x0001000C;

    /* Run training on slices 10-17 */
    dmc->DIRECT_ADDR = (DDR_ADDR_TRAIN_TYPE_RD_EYE |
                        (0x1FE00 << DDR_ADDR_DATA_SLICES_POS));
    dmc->DIRECT_CMD  = DDR_CMD_TRAIN_RANK_1;

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] MC initiated update...");

    dmc->DIRECT_ADDR = 0;
    dmc->DIRECT_CMD  = 0x0001000A;

    status = ddr_poll_training_status(dmc);
    if (status != FWK_SUCCESS)
        return status;

    dmc->DIRECT_CMD  = 0x0001000C;

    return FWK_SUCCESS;
}

static int dmc620_verify_phy_status(struct mod_dmc620_reg *dmc,
    fwk_id_t ddr_id)
{
    int status;
    uint8_t i;

    for (i = 0; i < 2; i++) {
        status = ddr_phy_api->verify_phy_status(ddr_id,
            DDR_ADDR_TRAIN_TYPE_WR_LVL);
        if (status != FWK_SUCCESS)
            return status;

        status = ddr_phy_api->verify_phy_status(ddr_id,
            DDR_ADDR_TRAIN_TYPE_RD_GATE);
        if (status != FWK_SUCCESS)
            return status;

        status = ddr_phy_api->verify_phy_status(ddr_id,
            DDR_ADDR_TRAIN_TYPE_RD_EYE);
        if (status != FWK_SUCCESS)
            return status;

        status = ddr_phy_api->verify_phy_status(ddr_id,
            DDR_ADDR_TRAIN_TYPE_VREF);
        if (status != FWK_SUCCESS)
            return status;
    }
    return FWK_SUCCESS;
}

static int dmc620_config(struct mod_dmc620_reg *dmc, fwk_id_t ddr_id)
{
    int status;

    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Initialising DMC620 at 0x%x\n", (uintptr_t)dmc);

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Writing functional settings\n");

    dmc->ADDRESS_CONTROL_NEXT = 0x00040502;
    dmc->DECODE_CONTROL_NEXT = 0x001C2800;
    dmc->FORMAT_CONTROL = 0x00000003;
    dmc->ADDRESS_MAP_NEXT = 0x00000003;
    dmc->ADDRESS_SHUTTER_31_00_NEXT = 0x00000000;
    dmc->ADDRESS_SHUTTER_63_32_NEXT = 0x00000000;
    dmc->ADDRESS_SHUTTER_95_64_NEXT = 0x00000000;
    dmc->ADDRESS_SHUTTER_127_96_NEXT = 0x00000000;
    dmc->ADDRESS_SHUTTER_159_128_NEXT = 0x00000000;
    dmc->ADDRESS_SHUTTER_191_160_NEXT = 0x00000000;
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
    dmc->ACCESS_ADDRESS_NEXT[0].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[1].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[2].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[3].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[4].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[5].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[6].MIN_43_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_NEXT[7].MIN_43_32 = 0x00000000;
    dmc->DCI_REPLAY_TYPE_NEXT = 0x00000000;
    dmc->DIRECT_CONTROL_NEXT = 0x00000000;
    dmc->DCI_STRB = 0x00000000;
    dmc->MEMORY_TYPE_NEXT = 0x00030102;
    dmc->FEATURE_CONFIG = 0x00001820;
    dmc->T_REFI_NEXT = 0x80000487;
    dmc->T_RFC_NEXT = 0x0005D9D2;
    dmc->T_MRR_NEXT = 0x00000001;
    dmc->T_MRW_NEXT = 0x00000018;
    dmc->T_RCD_NEXT = 0x00000014;
    dmc->T_RAS_NEXT = 0x00000027;
    dmc->T_RP_NEXT = 0x00000014;
    dmc->T_RPALL_NEXT = 0x00000013;
    dmc->T_RRD_NEXT = 0x04000604;
    dmc->T_ACT_WINDOW_NEXT = 0x00001017;
    dmc->T_RTR_NEXT = 0x14060604;
    dmc->T_RTW_NEXT = 0x001B1B1B;
    dmc->T_RTP_NEXT = 0x00000008;
    dmc->T_WR_NEXT = 0x00000029;
    dmc->T_WTR_NEXT = 0x001B1B1B;
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
    dmc->T_RDDATA_EN_NEXT = 0x0000000E;
    dmc->T_PHYRDLAT_NEXT = 0x00000046;
    dmc->T_PHYWRLAT_NEXT = 0x011f000C;
    dmc->RDLVL_CONTROL_NEXT = 0x01000000;
    dmc->RDLVL_MRS_NEXT = 0x00000224;
    dmc->T_RDLVL_RR_NEXT = 0x0000001E;
    dmc->WRLVL_CONTROL_NEXT = 0x00100000;
    dmc->WRLVL_MRS_NEXT = 0x00000181;
    dmc->T_WRLVL_EN_NEXT = 0x0000001E;
    dmc->PHY_UPDATE_CONTROL_NEXT = 0x01401111;
    dmc->T_LVL_DISCONNECT_NEXT = 0x00000001;
    dmc->WDQLVL_CONTROL_NEXT = 0x00000080;
    dmc->T_WDQLVL_EN_NEXT = 0x00000024;
    dmc->T_WDQLVL_WW_NEXT = 0x00000006;
    dmc->T_WDQLVL_RW_NEXT = 0x00000009;
    dmc->ERR0CTLR1 = 0x000000d1;
    dmc->RANK_REMAP_CONTROL_NEXT = 0x76543210;
    dmc->PHY_REQUEST_CS_REMAP = 0x76543210;
    dmc->T_ODTH_NEXT = 0x00000006;
    dmc->ODT_TIMING_NEXT = 0x06000600;
    dmc->T_RW_ODT_CLR_NEXT = 0x0000000f;
    dmc->T_CMD_NEXT = 0x00000000;
    dmc->T_RDLVL_EN_NEXT = 0x0000001E;
    dmc->T_WRLVL_WW_NEXT = 0x0000001E;
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
    dmc->T_RTR_NEXT = 0x14060604;
    dmc->T_DB_TRAIN_RESP_NEXT = 0x00000004;
    dmc->FEATURE_CONTROL_NEXT = 0x0aa30000;
    dmc->MUX_CONTROL_NEXT = 0x00000000;
    dmc->LOW_POWER_CONTROL_NEXT = 0x00000010;
    dmc->MEMORY_ADDRESS_MAX_31_00_NEXT = 0xffff001f;
    dmc->INTERRUPT_CONTROL = 0x00000070;

    dmc->DIRECT_CMD = 0x0001000C;

    dmc->USER_CONFIG0_NEXT = 0x1;
    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_GO;
    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) != MOD_DMC620_MEMC_CMD_GO)
        continue;

    status = ddr_phy_api->configure(ddr_id);
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

    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Sending direct DDR commands\n");

    direct_ddr_cmd(dmc);

    dmc620_config_interrupt(ddr_id);

    status = ddr_training(dmc);
    if (status != FWK_SUCCESS)
        return status;

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Enable DIMM refresh...");
    status = enable_dimm_refresh(dmc);
    if (status != FWK_SUCCESS)
        return status;

    /* Switch to READY */
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Setting DMC to READY mode\n");

    dmc->MEMC_CMD = MOD_DMC620_MEMC_CMD_GO;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_CMD) != MOD_DMC620_MEMC_CMD_GO)
        continue;

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] DMC init done.\n");

    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Verifying PHY status...");
    status = dmc620_verify_phy_status(dmc, ddr_id);
    if (status != FWK_SUCCESS)
        return status;
    log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    return FWK_SUCCESS;
}

static void execute_ddr_cmd(struct mod_dmc620_reg *dmc,
    uint32_t addr, uint32_t cmd)
{
    int status;

    dmc->DIRECT_ADDR = addr;
    dmc->DIRECT_CMD = cmd;
    status = dmc620_poll_dmc_status(dmc);
    if (status != FWK_SUCCESS) {
        log_api->log(MOD_LOG_GROUP_INFO,
            "[DDR] Execute command failed! ADDR: 0x%08x CMD: 0x%08x\n",
            addr, cmd);
    }
}

static void direct_ddr_cmd(struct mod_dmc620_reg *dmc)
{
    int count;

    execute_ddr_cmd(dmc, 0x00000004, 0x0001000A);
    execute_ddr_cmd(dmc, 0x00000006, 0x00010004);
    execute_ddr_cmd(dmc, 0x00000000, 0x0001000B);
    execute_ddr_cmd(dmc, 0x00000001, 0x0001000B);
    execute_ddr_cmd(dmc, 0x000003E8, 0x0001000D);
    execute_ddr_cmd(dmc, 0x00000258, 0x0001000D);
    execute_ddr_cmd(dmc, 0x00010001, 0x0001000B);
    execute_ddr_cmd(dmc, 0x0000002A, 0x0001000D);

    for (count = 0; count < 12; count++)
        execute_ddr_cmd(dmc, 0x00000200, 0x0001000D);

    execute_ddr_cmd(dmc, 0x000000A0, 0x0001070F);
    execute_ddr_cmd(dmc, 0x00000311, 0x0001070F);
    execute_ddr_cmd(dmc, 0x000000DC, 0x0001070F);
    execute_ddr_cmd(dmc, 0x000000EC, 0x0001070F);
    execute_ddr_cmd(dmc, 0x00000000, 0x00010000);

    /* MRS3 */
    execute_ddr_cmd(dmc, 0x00000220, 0x00010301);

    /* MRS6 */
    execute_ddr_cmd(dmc, 0x00000C97, 0x00010601);

    /* MRS5 */
    execute_ddr_cmd(dmc, 0x00000180, 0x00010501);

    /* MRS4 */
    execute_ddr_cmd(dmc, 0x00000000, 0x00010401);

    /* MRS2 */
    execute_ddr_cmd(dmc, 0x00000818, 0x00010201);

    /* MRS1 */
    execute_ddr_cmd(dmc, 0x00000003, 0x00010101);

    /* MRS0 */
    execute_ddr_cmd(dmc, 0x00000B40, 0x00010001);

    for (count = 0; count < 12; count++)
        execute_ddr_cmd(dmc, 0x00000200, 0x0001000D);

    execute_ddr_cmd(dmc, 0x00000400, 0x00010005);

    for (count = 0; count < 12; count++)
        execute_ddr_cmd(dmc, 0x00000200, 0x0001000D);
}

static int enable_dimm_refresh(struct mod_dmc620_reg *dmc)
{
    dmc->REFRESH_ENABLE_NEXT = 0x00000001;
    dmc->DIRECT_CMD  = 0x0001000C;

    return ddr_poll_training_status(dmc);
}

void dmc620_abort_recover(struct mod_dmc620_reg *dmc)
{
    uint32_t current_state;
    volatile uint32_t *dmc_abort = 0;

    current_state = dmc->MEMC_STATUS & 0x00000007;
    /* Make sure we don't run this from ABORT or RECOVERY states */
    if (current_state > 3) {
        log_api->log(MOD_LOG_GROUP_INFO,
            "[DDR] DMC generated abortable error from abort/recovery state\n");
        return;
    }

    /* Abort register is at offset 0x10000 */
    dmc_abort = (uint32_t *)((uint32_t)dmc + 0x10000);

    /* Assert abort request */
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Asserting abort request\n");
    *dmc_abort = 0x1;

    /* Wait for DMC to enter aborted state */
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Waiting for DMC to enter abort state...");
    while ((dmc->MEMC_STATUS & 0x00000007) != 0x4)
        continue;

    log_api->log(MOD_LOG_GROUP_INFO, "DONE\n");

    /* Deassert abort request */
    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Deasserting abort request\n");
    *dmc_abort = 0x0;

    /* Send ABORT_CLR command to change to recovery mode. */
    log_api->log(MOD_LOG_GROUP_INFO, "[DDR] Sending abort clear\n");
    dmc->MEMC_CMD = 0x00000006;

    /* Wait for state transition to complete */
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Waiting for DMC state transition...");
    while ((dmc->MEMC_STATUS & 0x00000007) != 0x5)
        continue;

    log_api->log(MOD_LOG_GROUP_INFO, "DONE\n");

    /* Go back to pre-error state */
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Initiating state transition back to normal world\n");
    dmc->MEMC_CMD = current_state;

    /* Wait for state transition to complete */
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Waiting for DMC state transition...");
    while ((dmc->MEMC_STATUS & 0x00000007) != current_state)
        continue;

    log_api->log(MOD_LOG_GROUP_INFO, "DONE\n");

    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Resuming operation in state %d\n", current_state);
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
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 MISC overflow interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MISC_OFLOW_IRQ);
}

void dmc0_err_oflow_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 error overflow interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_ERR_OFLOW_IRQ);
}

void dmc0_ecc_err_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 ECC error interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_ECC_ERR_INT_IRQ);
}

void dmc0_misc_access_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 misc access interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MISC_ACCESS_INT_IRQ);
}

void dmc0_temp_event_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 temperature event interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_TEMPERATURE_EVENT_INT_IRQ);
}

void dmc0_failed_access_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 failed access interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_FAILED_ACCESS_INT_IRQ);
}

void dmc0_mgr_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC0 mgr interrupt!\n");
    dmc620_handle_interrupt(0);
    fwk_interrupt_clear_pending(DMCS0_MGR_INT_IRQ);
}

void dmc1_misc_oflow_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 MISC overflow interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MISC_OFLOW_IRQ);
}

void dmc1_err_oflow_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 error overflow interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_ERR_OFLOW_IRQ);
}

void dmc1_ecc_err_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 ECC error interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_ECC_ERR_INT_IRQ);
}

void dmc1_misc_access_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 misc access interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MISC_ACCESS_INT_IRQ);
}

void dmc1_temp_event_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 temperature event interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_TEMPERATURE_EVENT_INT_IRQ);
}

void dmc1_failed_access_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 failed access interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_FAILED_ACCESS_INT_IRQ);
}

void dmc1_mgr_handler(void)
{
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] DMC1 mgr interrupt!\n");
    dmc620_handle_interrupt(1);
    fwk_interrupt_clear_pending(DMCS1_MGR_INT_IRQ);
}

static int dmc620_config_interrupt(fwk_id_t ddr_id)
{
    int id;

    id = fwk_id_get_element_idx(ddr_id);
    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR] Configuring interrupts for DMC%d\n", id);

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
