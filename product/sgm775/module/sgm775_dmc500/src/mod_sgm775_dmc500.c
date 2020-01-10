/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 DMC-500 module.
 */

#include <mod_sgm775_dmc500.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

#define COL_BITS 1
#define BANK_BITS 0
#define RANK_BITS 1
#define BANK_GROUP 0
#define ROW_BITS 4
#define MEM_TYPE 3
#define MEM_BURST 2
#define DEVICE_WIDTH 2
#define ADDR_SHUTTER 2

/* Timeout in us */
#define TIMEOUT_DMC_INIT_US (1000 * 1000)

static struct mod_timer_api *timer_api;
static struct mod_sgm775_dmc_ddr_phy_api *ddr_phy_api;

/* Forward declaration */
static int sgm775_dmc500_config(struct mod_sgm775_dmc500_reg *dmc,
                                fwk_id_t ddr_phy_id);

/*
 * Framework APIs
 */

static int mod_sgm775_dmc500_init(fwk_id_t module_id,
    unsigned int element_count, const void *data)
{
    return FWK_SUCCESS;
}

static int mod_sgm775_dmc500_element_init(fwk_id_t element_id,
    unsigned int unused, const void *data)
{
    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int mod_sgm775_dmc500_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_sgm775_dmc500_module_config *module_config;

    /* Nothing to do in the second round of calls. */
    if (round == 1)
        return FWK_SUCCESS;

    if (fwk_module_is_valid_module_id(id)) {

        module_config = fwk_module_get_data(fwk_module_id_sgm775_dmc500);
        fwk_assert(module_config != NULL);

        /* Bind to the timer */
        status = fwk_module_bind(module_config->timer_id,
            FWK_ID_API(FWK_MODULE_IDX_TIMER, 0), &timer_api);
        if (status != FWK_SUCCESS)
            return status;

        /* Bind to the DDR-PHY specific module */
        status =
            fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SGM775_DDR_PHY500),
                FWK_ID_API(FWK_MODULE_IDX_SGM775_DDR_PHY500, 0), &ddr_phy_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int mod_sgm775_dmc500_start(fwk_id_t id)
{
    const struct mod_sgm775_dmc500_element_config *element_config;
    struct mod_sgm775_dmc500_reg *dmc;

    /* Nothing to start for the module */
    if (fwk_module_is_valid_module_id(id))
        return FWK_SUCCESS;

    element_config = fwk_module_get_data(id);
    dmc = (struct mod_sgm775_dmc500_reg *)element_config->dmc;

    return sgm775_dmc500_config(dmc, element_config->ddr_phy_id);
}

const struct fwk_module module_sgm775_dmc500 = {
    .name = "SGM775_DMC500",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_sgm775_dmc500_init,
    .element_init = mod_sgm775_dmc500_element_init,
    .bind = mod_sgm775_dmc500_bind,
    .start = mod_sgm775_dmc500_start,
    .api_count = 0,
    .event_count = 0,
};

static int sgm775_dmc500_config(struct mod_sgm775_dmc500_reg *dmc,
                                fwk_id_t ddr_phy_id)
{
    int status;
    uint64_t timeout;
    uint64_t remaining_ticks;
    uint64_t counter;
    const struct mod_sgm775_dmc500_module_config *module_config;

    module_config = fwk_module_get_data(fwk_module_id_sgm775_dmc500);

    FWK_LOG_INFO("[DDR] Initialising DMC500 at 0x%x", (uintptr_t)dmc);

    dmc->ADDRESS_CONTROL = ((RANK_BITS << 24) |
                            (BANK_BITS << 16) |
                            (ROW_BITS << 8) |
                             COL_BITS);
    dmc->RANK_REMAP_CONTROL = 0x00000000;
    dmc->MEMORY_TYPE = ((BANK_GROUP << 16) |
                        (DEVICE_WIDTH << 8) |
                         MEM_TYPE);
    dmc->FORMAT_CONTROL = (MEM_BURST << 8);
    dmc->DECODE_CONTROL = 0x00000011;
    dmc->FEATURE_CONTROL = 0x00000000;
    dmc->ODT_WR_CONTROL_31_00 = 0x00000000;
    dmc->ODT_RD_CONTROL_31_00 = 0x00000000;
    dmc->ODT_TIMING = 0x10001000;

    FWK_LOG_INFO("[DDR] Setting timing settings");

    dmc->T_REFI = 0x0000030B;
    dmc->T_RFC = 0x000340D0;
    dmc->T_RDPDEN = 0x0000002E;
    dmc->T_RCD = 0x0000001D;
    dmc->T_RAS = 0x80000044;
    dmc->T_RP = 0x0000221D;
    dmc->T_RRD = 0x00001010;
    dmc->T_ACT_WINDOW = 0x00000040;
    dmc->T_RTR = 0x000C0808;
    dmc->T_RTW = 0x001F1F1F;
    dmc->T_RTP = 0x0000000C;
    dmc->T_WR = 0x00000035;
    dmc->T_WTR = 0x00082929;
    dmc->T_WTW = 0x000B0808;
    dmc->T_XTMW = 0x00000020;
    dmc->T_CLOCK_CONTROL = 0x1119030D;
    dmc->T_EP = 0x0000000C;
    dmc->T_XP = 0x000C000C;
    dmc->T_ESR = 0x00000019;
    dmc->T_XSR = 0x00E100E1;

    FWK_LOG_INFO("[DDR] Setting address map");

    dmc->ADDRESS_MAP = ((1 << 8) | (ADDR_SHUTTER));

    FWK_LOG_INFO("[DDR] Setting PMU settings");

    dmc->SI0_SI_INTERRUPT_CONTROL = 0x00000000;
    dmc->SI0_PMU_REQ_CONTROL = 0x00000B1A;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_0 = 0xB0562AA1;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_0 = 0xD0FB6716;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_1 = 0x7FC24C15;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_1 = 0xF7A9B2AC;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_2 = 0xDD35FA69;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_2 = 0x3555A8F5;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MASK_3 = 0xDE382B10;
    dmc->SI0_PMU_REQ_ATTRIBUTE_MATCH_3 = 0x3484B32C;
    dmc->SI0_THRESHOLD_CONTROL = 0x80000801;
    dmc->SI1_SI_INTERRUPT_CONTROL = 0x00000000;
    dmc->SI1_PMU_REQ_CONTROL = 0x00000B1A;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_0 = 0xB0562AA1;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_0 = 0xD0FB6716;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_1 = 0x7FC24C15;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_1 = 0xF7A9B2AC;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_2 = 0xDD35FA69;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_2 = 0x3555A8F5;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MASK_3 = 0xDE382B10;
    dmc->SI1_PMU_REQ_ATTRIBUTE_MATCH_3 = 0x3484B32C;
    dmc->SI1_THRESHOLD_CONTROL = 0x80000801;
    dmc->QUEUE_THRESHOLD_CONTROL_31_00 = 0xDEF8D550;
    dmc->QUEUE_THRESHOLD_CONTROL_63_32 = 0xB038362F;
    dmc->DCB_INTERRUPT_CONTROL = 0x00000000;
    dmc->PMU_DCB_CONTROL = 0x00000800;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_0 = 0xFD98CF7D;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_0 = 0x9F276EB5;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_1 = 0x40B1FC24;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_1 = 0x04BBF4FA;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_2 = 0x8089B0AF;
    dmc->PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_2 = 0x7D26E0BE;
    dmc->PMU_TAG_ENTRIES_ATTRIBUTE_MASK = 0x000000CE;
    dmc->PMU_TAG_ENTRIES_ATTRIBUTE_MATCH = 0x00000056;
    dmc->QE_INTERRUPT_CONTROL = 0x00000000;
    dmc->RANK_TURNAROUND_CONTROL = 0x8909020F;
    dmc->HIT_TURNAROUND_CONTROL = 0x37B8222C;
    dmc->QOS_CLASS_CONTROL = 0x00000D50;
    dmc->ESCALATION_CONTROL = 0x000D0C00;
    dmc->QV_CONTROL_31_00 = 0xED2626B0;
    dmc->QV_CONTROL_63_32 = 0x4159BE97;
    dmc->RT_CONTROL_31_00 = 0xE8DC790A;
    dmc->RT_CONTROL_63_32 = 0x9441A291;
    dmc->TIMEOUT_CONTROL = 0x00000003;
    dmc->WRITE_PRIORITY_CONTROL_31_00 = 0x81268C40;
    dmc->WRITE_PRIORITY_CONTROL_63_32 = 0x15F20D15;
    dmc->DIR_TURNAROUND_CONTROL = 0x06060403;
    dmc->HIT_PREDICTION_CONTROL = 0x00020705;
    dmc->REFRESH_PRIORITY = 0x00000204;
    dmc->MC_UPDATE_CONTROL = 0x0000FF00;
    dmc->PHY_UPDATE_CONTROL = 0x15A3925F;
    dmc->PHY_MASTER_CONTROL = 0x6875AF9A;
    dmc->LOW_POWER_CONTROL = 0x000E0801;
    dmc->PMU_QE_CONTROL = 0x00000C0D;
    dmc->PMU_QE_MUX = 0x05670023;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MASK_0 = 0x000000F1;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_0 = 0x00000662;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MASK_1 = 0x000000DD;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_1 = 0x00000097;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MASK_2 = 0x0000001A;
    dmc->PMU_QOS_ENGINE_ATTRIBUTE_MATCH_2 = 0x00000755;
    dmc->PMU_QUEUED_ENTRIES_ATTRIBUTE_MASK = 0xAD625ED5;
    dmc->PMU_QUEUED_ENTRIES_ATTRIBUTE_MATCH = 0x853C65BB;
    dmc->MI_INTERRUPT_CONTROL = 0x00000000;
    dmc->POWER_DOWN_CONTROL = 0x00000005;
    dmc->REFRESH_CONTROL = 0x00000000;
    dmc->PMU_MI_CONTROL = 0x00000100;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MASK_0 = 0x0032BB0E;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MATCH_0 = 0x0033F5AB;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MASK_1 = 0x00296B28;
    dmc->PMU_MEMORY_IF_ATTRIBUTE_MATCH_1 = 0x002C67BF;
    dmc->PMU_BANK_STATES_ATTRIBUTE_MASK = 0x00000005;
    dmc->PMU_BANK_STATES_ATTRIBUTE_MATCH = 0x00000019;
    dmc->PMU_RANK_STATES_ATTRIBUTE_MASK = 0x0000001B;
    dmc->PMU_RANK_STATES_ATTRIBUTE_MATCH = 0x00000020;
    dmc->CFG_INTERRUPT_CONTROL = 0x00000000;
    dmc->T_RDDATA_EN = 0x00000001;
    dmc->T_PHYRDLAT = 0x0000003F;
    dmc->T_PHYWRLAT = 0x010F170E;
    dmc->ERR_RAMECC_CTLR = 0x00000000;

    FWK_LOG_INFO("[DDR] Setting PHY-related settings");

    dmc->PHY_POWER_CONTROL = 0x0000012A;
    dmc->T_PHY_TRAIN = 0x00F8000A;
    dmc->PHYUPD_INIT = 0x00000000;
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

    dmc->DIRECT_CMD_SETTINGS = 0x00C80000;
    dmc->DIRECT_CMD = 0x00000000;
    dmc->DIRECT_CLK_DISABLE = 0x00280003;
    dmc->CLK_STATUS_OVERRIDE = 0x00000003;
    dmc->DIRECT_CMD_SETTINGS = 0x01F40003;
    dmc->DIRECT_CMD = 0x00800080;
    dmc->RANK_STATUS_OVERRIDE = 0x30000003;
    dmc->DIRECT_CMD_SETTINGS = 0x04B00003;
    dmc->DIRECT_CMD = 0x00800FE0;
    dmc->DIRECT_CMD_SETTINGS = 0x00500003;
    dmc->DIRECT_CMD = 0x008011E0;
    dmc->DIRECT_CMD_SETTINGS = 0x00140003;
    dmc->DIRECT_CMD = 0x06D601C6;
    dmc->DIRECT_CMD_SETTINGS = 0x01000003;
    dmc->DIRECT_CMD = 0x00F60DC6;
    dmc->DIRECT_CMD_SETTINGS = 0x00140003;
    dmc->DIRECT_CMD = 0x31D603C6;
    dmc->DIRECT_CMD_SETTINGS = 0x00140003;
    dmc->DIRECT_CMD = 0x16F601C6;
    dmc->DIRECT_CMD_SETTINGS = 0x00140003;
    dmc->DIRECT_CMD = 0x2DD602C6;
    dmc->DIRECT_CMD_SETTINGS = 0x02000003;
    dmc->DIRECT_CMD = 0x00D60DE6;
    dmc->REFRESH_ENABLE = 0x00000001;

    FWK_LOG_INFO("[DDR] Setting dmc in READY mode");

    status = timer_api->time_to_timestamp(module_config->timer_id,
                                          TIMEOUT_DMC_INIT_US, &timeout);
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

    dmc->MI_STATE_CONTROL = 0x00000000;
    dmc->QUEUE_STATE_CONTROL = 0x00000000;
    dmc->SI0_SI_STATE_CONTROL = 0x00000000;
    dmc->SI1_SI_STATE_CONTROL = 0x00000000;

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
