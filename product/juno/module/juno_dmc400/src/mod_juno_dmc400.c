/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno DMC-400 driver
 */

#include "juno_dmc400.h"
#include "juno_id.h"
#include "juno_irq.h"
#include "juno_scc.h"
#include "scp_config.h"
#include "system_clock.h"

#include <mod_juno_dmc400.h>
#include <mod_power_domain.h>
#include <mod_system_power.h>
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
#include <fwk_thread.h>

#include <fmw_cmsis.h>

#include <stddef.h>

struct juno_dmc400_ctx {
    const struct mod_juno_dmc400_ddr_phy_api *ddr_phy_api;
    const struct mod_timer_api *timer_api;
    unsigned int dmc_refclk_ratio;
};

static struct juno_dmc400_ctx ctx;

#define DDR_CHANNEL_COUNT           2

/*
 * Static helpers
 */

static bool ddr_clock_div_set_check(void *data)
{
    return ((SCP_CONFIG->DMCCLK_CONTROL & DMCCLK_CONTROL_CRNTCLKDIV) ==
        (CLKDIV << 16));
}

static bool ddr_clock_sel_set_check(void *data)
{
    return ((SCP_CONFIG->DMCCLK_CONTROL & DMCCLK_CONTROL_CRNTCLK) ==
        (DMCCLK_CONTROL_CLKSEL_SYSINCLK << 12));
}

static bool ddr_clock_enable_check(void *data)
{
    uint32_t clock_enable_status;

    clock_enable_status = SCP_CONFIG->CLOCK_ENABLE_STATUS;

    return ((clock_enable_status & SCP_CONFIG_CLOCK_ENABLE_DMCCLKEN) != 0);
}

static bool dmc_user_config_dfi_check(void *data)
{
    const struct mod_juno_dmc400_reg *dmc = data;
    uint32_t init_complete_mask;

    init_complete_mask = (DMC_USER_STATUS_DFI_INIT_COMPLETE_0 |
                          DMC_USER_STATUS_DFI_INIT_COMPLETE_1);

    return ((dmc->USER_STATUS & init_complete_mask) == init_complete_mask);
}

static bool ddr_cmd_config_check(void *data)
{
    const struct mod_juno_dmc400_reg *dmc = data;

    return (dmc->MEMC_STATUS == DMC400_CMD_CONFIG);
}

static bool ddr_cmd_go_check(void *data)
{
    const struct mod_juno_dmc400_reg *dmc = data;

    return (dmc->MEMC_STATUS == DMC400_CMD_GO);
}

static bool ddr_cmd_sleep_check(void *data)
{
    const struct mod_juno_dmc400_reg *dmc = data;

    return (dmc->MEMC_STATUS == DMC400_CMD_SLEEP);
}

static bool ddr_cmd_low_power_check(void *data)
{
    const struct mod_juno_dmc400_reg *dmc = data;

    return ((dmc->MEMC_STATUS & DMC_MEMC_STATUS) == DMC400_CMD_SLEEP);
}

static void dmc_delay_cycles(uint32_t dmc_cycles)
{
    unsigned int i;

    for (i = 0; i < (dmc_cycles / ctx.dmc_refclk_ratio); i++)
        __NOP();
}

static void init_ddr_chip(struct mod_juno_dmc400_reg *dmc, uint32_t dev)
{
    dmc->DIRECT_CMD = dev;
    dmc_delay_cycles(20);
    dmc->DIRECT_CMD = 0x10020018 | dev;
    dmc_delay_cycles(1);
    dmc->DIRECT_CMD = 0x10030000 | dev;
    dmc_delay_cycles(1);
    dmc->DIRECT_CMD = 0x10010046 | dev;
    dmc_delay_cycles(1);
    dmc->DIRECT_CMD = 0x10000D70 | dev;
    dmc_delay_cycles(60);
    dmc->DIRECT_CMD = 0x50000400 | dev;
    dmc_delay_cycles(1);
}

static int ddr_clock_div_set(fwk_id_t timer_id)
{
    fwk_assert(CLKDIV <= 16);
    fwk_assert(CLKDIV != 0);

    SCP_CONFIG->DMCCLK_CONTROL =
        (SCP_CONFIG->DMCCLK_CONTROL & ~DMCCLK_CONTROL_CLKDIV) | (CLKDIV << 4);

    return ctx.timer_api->wait(timer_id,
                               DMC400_CLOCK_DIV_SET_WAIT_TIMEOUT_US,
                               ddr_clock_div_set_check,
                               NULL);
}

static int ddr_clock_sel_set(fwk_id_t timer_id)
{
    SCP_CONFIG->DMCCLK_CONTROL =
        ((SCP_CONFIG->DMCCLK_CONTROL & ~DMCCLK_CONTROL_CLKSEL) |
        DMCCLK_CONTROL_CLKSEL_SYSINCLK);

    return ctx.timer_api->wait(timer_id,
                               DMC400_CLOCK_SEL_SET_WAIT_TIMEOUT_US,
                               ddr_clock_sel_set_check,
                               NULL);
}

static int ddr_clock_enable(fwk_id_t timer_id)
{
    SCP_CONFIG->CLOCK_ENABLE_SET |= SCP_CONFIG_CLOCK_ENABLE_DMCCLKEN;

    return ctx.timer_api->wait(timer_id,
                               DMC400_CLOCK_ENABLE_WAIT_TIMEOUT_US,
                               ddr_clock_enable_check,
                               NULL);
}

static void ddr_remove_poreset(struct mod_juno_dmc400_reg *dmc)
{
    dmc->USER_CONFIG0 |=
        DMC_USER_CONFIG_CORE_PRSTN | DMC_USER_CONFIG_CORE_SRSTN;
    dmc->USER_CONFIG1 |=
        DMC_USER_CONFIG_CORE_PRSTN | DMC_USER_CONFIG_CORE_SRSTN;

    dmc_delay_cycles(DELAY_DDR_PORESET_CYCLES);
}

static int ddr_phy_init(fwk_id_t id)
{
    int status;
    const struct mod_juno_dmc400_element_config *element_config;
    const struct mod_juno_dmc400_module_config *module_config;
    struct mod_juno_dmc400_reg *dmc;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    element_config = fwk_module_get_data(id);
    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;

    status = ctx.ddr_phy_api->configure_ddr(element_config->ddr_phy_0_id);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx.ddr_phy_api->configure_ddr(element_config->ddr_phy_1_id);
    if (status != FWK_SUCCESS)
        return status;

    /* Initialize PHYs */
    dmc->USER_CONFIG0 |= DMC_USER_CONFIG_DFI_INIT_START;
    dmc->USER_CONFIG1 |= DMC_USER_CONFIG_DFI_INIT_START;

    if (!element_config->is_platform_fvp) {
        /* Wait for the PHYs initialization to complete */
        status = ctx.timer_api->wait(module_config->timer_id,
                                     DMC400_PHY_INIT_WAIT_TIMEOUT_US,
                                     dmc_user_config_dfi_check,
                                     dmc);
        if (status != FWK_SUCCESS)
            return status;
    }

    /* Remove initialization request */
    dmc->USER_CONFIG0 &= ~DMC_USER_CONFIG_DFI_INIT_START;
    dmc->USER_CONFIG1 &= ~DMC_USER_CONFIG_DFI_INIT_START;

    return FWK_SUCCESS;
}

static int ddr_clk_init(fwk_id_t id)
{
    int status;
    const struct mod_juno_dmc400_module_config *module_config;
    const struct mod_juno_dmc400_element_config *element_config;
    struct mod_juno_dmc400_reg *dmc;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    element_config = fwk_module_get_data(id);
    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;

    /* Clock divider */
    status = ddr_clock_div_set(module_config->timer_id);
    if (status != FWK_SUCCESS)
        return status;

    /* Clock source */
    status = ddr_clock_sel_set(module_config->timer_id);
    if (status != FWK_SUCCESS)
        return status;

    /* Clock enable */
    status = ddr_clock_enable(module_config->timer_id);
    if (status != FWK_SUCCESS)
        return status;

    /* Set DDR PHY PLLs after DMCCLK is stable */
    status = ctx.ddr_phy_api->configure_clk(fwk_module_id_juno_ddr_phy400);
    if (status != FWK_SUCCESS)
        return status;

    ddr_remove_poreset(dmc);

    return FWK_SUCCESS;
}

static int ddr_dmc_init(fwk_id_t id)
{
    uint32_t ddr_chip_count, chip, channel, dev;
    const struct mod_juno_dmc400_element_config *element_config;
    struct mod_juno_dmc400_reg *dmc;

    element_config = fwk_module_get_data(id);
    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;

    /* QoS control */
    dmc->TURNAROUND_PRIORITY = 0x0000008C;
    dmc->HIT_PRIORITY = 0x00000000;
    dmc->QOS0_CONTROL = 0x00000000;
    dmc->QOS1_CONTROL = 0x00000001;
    dmc->QOS2_CONTROL = 0x00000002;
    dmc->QOS3_CONTROL = 0x00000003;
    dmc->QOS4_CONTROL = 0x00000004;
    dmc->QOS5_CONTROL = 0x00000005;
    dmc->QOS6_CONTROL = 0x00000006;
    dmc->QOS7_CONTROL = 0x00000007;
    dmc->QOS8_CONTROL = 0x00000008;
    dmc->QOS9_CONTROL = 0x00000009;
    dmc->QOS10_CONTROL = 0x0000000A;
    dmc->QOS11_CONTROL = 0x0000000B;
    dmc->QOS12_CONTROL = 0x0000000C;
    dmc->QOS13_CONTROL = 0x0000000D;
    dmc->QOS14_CONTROL = 0x0000000E;
    dmc->QOS15_CONTROL = 0x0000000F;
    dmc->TIMEOUT_CONTROL = 0x00000002;
    dmc->QUEUE_CONTROL = 0x00000000;
    dmc->WRITE_PRIORITY_CONTROL = 0x00000001;
    dmc->WRITE_PRIORITY_CONTROL2 = 0xBBBB8888;
    dmc->READ_PRIORITY_CONTROL = 0x88000002;
    dmc->READ_PRIORITY_CONTROL2 = 0xCCCCCCCC;

    dmc->ACCESS_ADDRESS_MATCH_31_00 = 0x00000000;
    dmc->ACCESS_ADDRESS_MATCH_63_32 = 0x00000000;
    dmc->ACCESS_ADDRESS_MASK_31_00 = 0x00000000;
    dmc->ACCESS_ADDRESS_MASK_63_32 = 0x000000FE;
    dmc->REFRESH_CONTROL = 0x00000000;
    dmc->INTERRUPT_CONTROL = 0x00000000;
    dmc->INTERRUPT_CLR = 0x00000000;
    dmc->FORMAT_CONTROL = 0x11000103;

    /* Timings frequency-dependent */
    dmc->T_LPRESP = 0x00000007;
    dmc->T_REFI = 0x00000186;
    dmc->T_RFC = 0x008C008C;
    dmc->T_MRR = 0x00000002;
    dmc->T_MRW = 0x00000007;
    dmc->T_RCD = 0x00000006;
    dmc->T_RAS = 0x0000000F;
    dmc->T_RP = 0x00000006;
    dmc->T_RPALL = 0x00000007;
    dmc->T_RRD = 0x00000004;
    dmc->T_FAW = 0x00000011;
    dmc->T_RTR = 0x0000000B;
    dmc->T_RTW = 0x00000007;
    dmc->T_RTP = 0x00000004;
    dmc->T_WR = 0x0000000D;
    dmc->T_WTR = 0x0004000A;
    dmc->T_WTW = 0x000C0000;
    dmc->READ_LATENCY = 0x0000000C;
    dmc->WRITE_LATENCY = 0x00000009;
    dmc->T_RDDATA_EN = 0x00000003;
    dmc->T_PHYWRLAT = 0x00000001;
    dmc->WRLVL_MRS = 0x000000C6;

    /* Set sync-up bridge */
    dmc->USER_CONFIG0 |=
        DMC_USER_CONFIG_TWRDATA_DELAY | DMC_USER_CONFIG_TWRDATA_EN_DELAY;
    dmc->USER_CONFIG1 |=
        DMC_USER_CONFIG_TWRDATA_DELAY | DMC_USER_CONFIG_TWRDATA_EN_DELAY;

    dmc->T_EP = 0x00000004;
    dmc->T_XP = 0x000B0004;
    dmc->T_ESR = 0x00000004;
    dmc->T_XSR = 0x01000090;
    dmc->T_SRCKD = 0x00000005;
    dmc->T_CKSRD = 0x00000005;
    dmc->T_ECKD = 0x00000005;
    dmc->T_XCKD = 0x00000005;

    /* General control */
    dmc->ADDRESS_CONTROL = element_config->address_control;

    dmc->DECODE_CONTROL = 0x10000043;
    dmc->MODE_CONTROL = 0x00000012;
    dmc->LOW_POWER_CONTROL = 0x00000010;

    /*  Direct commands to initialize LPDDR3 */
    ddr_chip_count = element_config->ddr_chip_count;
    for (channel = 0; channel < DDR_CHANNEL_COUNT; channel++) {
        for (chip = 0; chip < ddr_chip_count; chip++) {
            dev = (channel << 24) | (chip << 20);

            init_ddr_chip(dmc, dev);
        }
    }

    dmc_delay_cycles(DELAY_DDR_INIT_LPDDR3_CYCLES);

    /* Auto refresh */
    dmc->DIRECT_CMD = DIRECT_CMD_AUTOREFRESH |
                      DIRECT_CMD_CHANNEL_0_ADDR |
                      DIRECT_CMD_CHIP_0_ADDR;

    if (element_config->ddr_chip_count > 1) {
        dmc->DIRECT_CMD = DIRECT_CMD_AUTOREFRESH |
                          DIRECT_CMD_CHANNEL_0_ADDR |
                          DIRECT_CMD_CHIP_1_ADDR;
    }

    dmc->DIRECT_CMD = DIRECT_CMD_AUTOREFRESH |
                      DIRECT_CMD_CHANNEL_1_ADDR |
                      DIRECT_CMD_CHIP_0_ADDR;

    if (element_config->ddr_chip_count > 1) {
        dmc->DIRECT_CMD = DIRECT_CMD_AUTOREFRESH |
                          DIRECT_CMD_CHANNEL_1_ADDR |
                          DIRECT_CMD_CHIP_1_ADDR;
    }

    return FWK_SUCCESS;
}

/*
 * The training sequence must run with interrupts disabled
 */
static int ddr_training(fwk_id_t id)
{
    int status;
    uint64_t timeout, remaining_ticks, counter;
    const struct mod_juno_dmc400_element_config *element_config;
    const struct mod_juno_dmc400_module_config *module_config;
    struct mod_juno_dmc400_reg *dmc;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    element_config = fwk_module_get_data(id);
    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;

    if (element_config->is_platform_fvp) {
        /* Training unsupported on FVP */
        return FWK_SUCCESS;
    }

    /* Configure the time-out for the DDR programming */
    status = ctx.timer_api->time_to_timestamp(module_config->timer_id,
                                              DMC400_TRAINING_TIMEOUT_US,
                                              &timeout);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx.timer_api->get_counter(module_config->timer_id, &counter);
    if (status != FWK_SUCCESS)
        return status;

    timeout += counter;

    /* Training related setup */
    dmc->RDLVL_CONTROL = 0x00001002;
    dmc->T_RDLVL_EN = 0x00000020;
    dmc->T_RDLVL_RR = 0x00000014;
    dmc->WRLVL_CONTROL = 0x00001002;
    dmc->WRLVL_DIRECT = 0x00000000;
    dmc->T_WRLVL_EN = 0x00000028;
    dmc->T_WRLVL_WW = 0x0000001B;

    /*
     * Write training
     */

    /* Channel 0, chip 0 */
    dmc->WRLVL_DIRECT = WRITE_LEVELING_REQUEST_SEND |
                        WRITE_LEVELING_CHANNEL_0 |
                        WRITE_LEVELING_CHIP_0;
    while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M0_MASK) !=
        DMC_CHANNEL_STATUS_M0_IDLE) {
        status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                          &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;
        if (remaining_ticks == 0)
            goto timeout;
    }

    /* Channel 1, chip 0 */
    dmc->WRLVL_DIRECT = WRITE_LEVELING_REQUEST_SEND |
                        WRITE_LEVELING_CHANNEL_1 |
                        WRITE_LEVELING_CHIP_0;
    while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M1_MASK) !=
        DMC_CHANNEL_STATUS_M1_IDLE) {
        status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                          &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;
        if (remaining_ticks == 0)
            goto timeout;
    }

    if (element_config->ddr_chip_count > 1) {
        /* Channel 0, chip 1 */
        dmc->WRLVL_DIRECT = WRITE_LEVELING_REQUEST_SEND |
                            WRITE_LEVELING_CHANNEL_0 |
                            WRITE_LEVELING_CHIP_1;
        while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M0_MASK) !=
            DMC_CHANNEL_STATUS_M0_IDLE) {
            status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                              &remaining_ticks);
            if (status != FWK_SUCCESS)
                return status;
            if (remaining_ticks == 0)
                goto timeout;
        }

        /* Channel 1, chip 1 */
        dmc->WRLVL_DIRECT = WRITE_LEVELING_REQUEST_SEND |
                            WRITE_LEVELING_CHANNEL_1 |
                            WRITE_LEVELING_CHIP_1;
        while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M1_MASK) !=
            DMC_CHANNEL_STATUS_M1_IDLE) {
            status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                              &remaining_ticks);
            if (status != FWK_SUCCESS)
                return status;
            if (remaining_ticks == 0)
                goto timeout;
        }
    }

    /*
     * Read Gate training
     */

    /* Channel 0, chip 0 */
    dmc->RDLVL_DIRECT = READ_GATE_TRAINING_REQUEST_SEND |
                        READ_LEVELING_CHANNEL_0 |
                        READ_LEVELING_CHIP_0;
    while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M0_MASK) !=
        DMC_CHANNEL_STATUS_M0_IDLE) {
        status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                          &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;
        if (remaining_ticks == 0)
            goto timeout;
    }

    /* Channel 1, chip 0 */
    dmc->RDLVL_DIRECT = READ_GATE_TRAINING_REQUEST_SEND |
                        READ_LEVELING_CHANNEL_1 |
                        READ_LEVELING_CHIP_0;
    while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M1_MASK) !=
        DMC_CHANNEL_STATUS_M1_IDLE) {
        status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                          &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;
        if (remaining_ticks == 0)
            goto timeout;
    }

    if (element_config->ddr_chip_count > 1) {
        /* Channel 0, chip 1 */
        dmc->RDLVL_DIRECT = READ_GATE_TRAINING_REQUEST_SEND |
                            READ_LEVELING_CHANNEL_0 |
                            READ_LEVELING_CHIP_1;
        while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M0_MASK) !=
            DMC_CHANNEL_STATUS_M0_IDLE) {
            status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                              &remaining_ticks);
            if (status != FWK_SUCCESS)
                return status;
            if (remaining_ticks == 0)
                goto timeout;
        }

        /* Channel 1, chip 1 */
        dmc->RDLVL_DIRECT = READ_GATE_TRAINING_REQUEST_SEND |
                            READ_LEVELING_CHANNEL_1 |
                            READ_LEVELING_CHIP_1;
        while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M1_MASK) !=
            DMC_CHANNEL_STATUS_M1_IDLE) {
            status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                              &remaining_ticks);
            if (status != FWK_SUCCESS)
                return status;
            if (remaining_ticks == 0)
                goto timeout;
        }
    }

    /*
     * Read Eye training
     */

    /* Channel 0, chip 0 */
    dmc->RDLVL_DIRECT = READ_EYE_TRAINING_REQUEST_SEND |
                        READ_LEVELING_CHANNEL_0 |
                        READ_LEVELING_CHIP_0;
    while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M0_MASK) !=
        DMC_CHANNEL_STATUS_M0_IDLE) {
        status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                          &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;
        if (remaining_ticks == 0)
            goto timeout;
    }

    /* Channel 1, chip 0 */
    dmc->RDLVL_DIRECT = READ_EYE_TRAINING_REQUEST_SEND |
                        READ_LEVELING_CHANNEL_1 |
                        READ_LEVELING_CHIP_0;
    while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M1_MASK) !=
        DMC_CHANNEL_STATUS_M1_IDLE) {
        status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                          &remaining_ticks);
        if (status != FWK_SUCCESS)
            return status;
        if (remaining_ticks == 0)
            goto timeout;
    }

    if (element_config->ddr_chip_count > 1) {
        /* Channel 0, chip 1 */
        dmc->RDLVL_DIRECT = READ_EYE_TRAINING_REQUEST_SEND |
                            READ_LEVELING_CHANNEL_0 |
                            READ_LEVELING_CHIP_1;
        while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M0_MASK) !=
            DMC_CHANNEL_STATUS_M0_IDLE) {
            status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                              &remaining_ticks);
            if (status != FWK_SUCCESS)
                return status;
            if (remaining_ticks == 0)
                goto timeout;
        }

        /* Channel 1, chip 1 */
        dmc->RDLVL_DIRECT = READ_EYE_TRAINING_REQUEST_SEND |
                            READ_LEVELING_CHANNEL_1 |
                            READ_LEVELING_CHIP_1;
        while ((dmc->CHANNEL_STATUS & DMC_CHANNEL_STATUS_M1_MASK) !=
            DMC_CHANNEL_STATUS_M1_IDLE) {
            status = ctx.timer_api->remaining(module_config->timer_id, timeout,
                                              &remaining_ticks);
            if (status != FWK_SUCCESS)
                return status;
            if (remaining_ticks == 0)
                goto timeout;
        }

    }

    return FWK_SUCCESS;

timeout:
    FWK_LOG_WARN("[DMC] Training time-out");

    return FWK_E_TIMEOUT;
}

static int ddr_retraining(fwk_id_t id)
{
    int status;
    const struct mod_juno_dmc400_element_config *element_config;
    const struct mod_juno_dmc400_module_config *module_config;
    struct mod_juno_dmc400_reg *dmc;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    element_config = fwk_module_get_data(id);
    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;

    fwk_interrupt_global_disable();

    dmc->MEMC_CMD = DMC400_CMD_CONFIG;
    status = ctx.timer_api->wait(module_config->timer_id,
                                 DMC400_CONFIG_WAIT_TIMEOUT_US,
                                 ddr_cmd_config_check,
                                 dmc);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_training(id);
    if (status != FWK_SUCCESS)
        return status;

    dmc->MEMC_CMD = DMC400_CMD_GO;

    fwk_interrupt_global_enable();

    fwk_interrupt_enable(PHY_TRAINING_IRQ);

    FWK_LOG_INFO("[DMC] Re-training done");

    return FWK_SUCCESS;
}

static int ddr_wake(fwk_id_t id)
{
    int status;
    uint32_t ddr_chip_count, chip, channel, dev;
    const struct mod_juno_dmc400_element_config *element_config;
    const struct mod_juno_dmc400_module_config *module_config;
    struct mod_juno_dmc400_reg *dmc;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    element_config = fwk_module_get_data(id);
    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;

    dmc_delay_cycles(64);

    /* Disable PHY retention */
    status = ctx.ddr_phy_api->configure_retention(fwk_module_id_juno_ddr_phy400,
                                                  false);
    if (status != FWK_SUCCESS)
        return status;

    dmc_delay_cycles(64);

    /*
     * The DDR modules need to be sent a NOP during the transition
     * from DMC sleep to DMC configure modes in order to disable
     * self-refresh.
     */
    dmc->MEMC_CMD = DMC400_CMD_CONFIG;
    status = ctx.timer_api->wait(module_config->timer_id,
                                 DMC400_CONFIG_WAIT_TIMEOUT_US,
                                 ddr_cmd_config_check,
                                 dmc);
    if (status != FWK_SUCCESS)
        return status;

    ddr_chip_count = element_config->ddr_chip_count;
    for (channel = 0; channel < DDR_CHANNEL_COUNT; channel++) {
        for (chip = 0; chip < ddr_chip_count; chip++) {
            dev = (channel << 24) | (chip << 20);

            dmc->DIRECT_CMD = dev;
            dmc_delay_cycles(20);
        }
    }

    dmc->MEMC_CMD = DMC400_CMD_GO;
    status = ctx.timer_api->wait(module_config->timer_id,
                                 DMC400_CONFIG_WAIT_TIMEOUT_US,
                                 ddr_cmd_go_check,
                                 dmc);
    if (status != FWK_SUCCESS)
        return status;

    dmc_delay_cycles(64);

    /* Re-run training program */
    return ddr_retraining(id);
}

static int ddr_resume(const struct mod_juno_dmc400_element_config *config,
                      fwk_id_t id)
{
    int status;
    const struct mod_juno_dmc400_module_config *module_config;
    struct mod_juno_dmc400_reg *dmc;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    dmc = (struct mod_juno_dmc400_reg *)config->dmc;

    status = ddr_clk_init(id);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_phy_init(id);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_dmc_init(id);
    if (status != FWK_SUCCESS)
        return status;

    /* Sleep the DMC */
    dmc->MEMC_CMD = DMC400_CMD_SLEEP;
    status = ctx.timer_api->wait(module_config->timer_id,
                                 DMC400_CONFIG_WAIT_TIMEOUT_US,
                                 ddr_cmd_sleep_check,
                                 dmc);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_wake(id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_notification_unsubscribe(
        mod_pd_notification_id_power_state_transition,
        config->pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_notification_subscribe(
        mod_pd_notification_id_power_state_pre_transition,
        config->pd_id,
        id);
}

static int ddr_suspend(const struct mod_juno_dmc400_element_config *config,
                       fwk_id_t id)
{
    int status;
    struct mod_juno_dmc400_reg *dmc;
    const struct mod_juno_dmc400_module_config *module_config;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    dmc = (struct mod_juno_dmc400_reg *)config->dmc;

    status = fwk_interrupt_disable(PHY_TRAINING_IRQ);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    /* Set low power mode */
    dmc->MEMC_CMD = DMC400_CMD_SLEEP;
    status = ctx.timer_api->wait(module_config->timer_id,
                                 DMC400_CONFIG_WAIT_TIMEOUT_US,
                                 ddr_cmd_low_power_check,
                                 dmc);

    dmc_delay_cycles(64);

    /* Wait for the PHYs to be idle */
    status = ctx.ddr_phy_api->configure_idle(config->ddr_phy_0_id);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx.ddr_phy_api->configure_idle(config->ddr_phy_1_id);
    if (status != FWK_SUCCESS)
        return status;

    /* Enable PHY retention */
    status = ctx.ddr_phy_api->configure_retention(fwk_module_id_juno_ddr_phy400,
                                                  true);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_notification_unsubscribe(
        mod_pd_notification_id_power_state_pre_transition,
        config->pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        config->pd_id,
        id);
}

static void ddr_phy_irq_handler(void)
{
    int status;
    struct fwk_event req_event;
    enum juno_idx_revision revision = JUNO_IDX_REVISION_COUNT;
    fwk_id_t id = FWK_ID_ELEMENT(FWK_MODULE_IDX_JUNO_DMC400, 0);

    status = juno_id_get_revision(&revision);
    fwk_assert(status == FWK_SUCCESS);

    /*
     * Retrain can be requested only by Juno R0
     */
    fwk_assert(revision == JUNO_IDX_REVISION_R0);

    fwk_interrupt_disable(PHY_TRAINING_IRQ);

    req_event = (struct fwk_event) {
        .source_id = id,
        .target_id = id,
        .id = juno_dmc400_event_id_training,
    };

    status = fwk_thread_put_event(&req_event);
    fwk_assert(status == FWK_SUCCESS);
}

/*
 * Framework handlers
 */

static int juno_dmc400_init(fwk_id_t module_id,
                            unsigned int element_count,
                            const void *data) {

    fwk_assert(data != NULL);
    fwk_assert(element_count == 1);

    return FWK_SUCCESS;
}

static int juno_dmc400_element_init(fwk_id_t element_id,
                                    unsigned int sub_element_count,
                                    const void *data) {

    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int juno_dmc400_bind(fwk_id_t id, unsigned int round)
{

    int status;
    const struct mod_juno_dmc400_module_config *module_config;

    /* Nothing to do in the second round of calls */
    if (round >= 1)
        return FWK_SUCCESS;

    /* Nothing to do in case of elements */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    module_config = fwk_module_get_data(fwk_module_id_juno_dmc400);
    fwk_assert(module_config != NULL);

    status = fwk_module_bind(module_config->ddr_phy_module_id,
                             module_config->ddr_phy_api_id, &ctx.ddr_phy_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(module_config->timer_id,
                             FWK_ID_API(FWK_MODULE_IDX_TIMER, 0),
                             &ctx.timer_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int juno_dmc400_start(fwk_id_t id)
{
    int status;
    const struct mod_juno_dmc400_element_config *element_config;
    struct mod_juno_dmc400_reg *dmc;

    /* Nothing to start for the module */
    if (fwk_module_is_valid_module_id(id))
        return FWK_SUCCESS;

    if (SCC->GPR0 & SCC_GPR0_DDR_DISABLE) {
        FWK_LOG_INFO("[DMC] GPR_0 disable flag set: skipping init");

        return FWK_SUCCESS;
    }

    element_config = fwk_module_get_data(id);

    dmc = (struct mod_juno_dmc400_reg *)element_config->dmc;
    if (!fwk_expect(dmc != NULL))
        return FWK_E_PARAM;

    fwk_assert(CLKDIV > 0);

    ctx.dmc_refclk_ratio = (DDR_FREQUENCY_MHZ * FWK_MHZ) / CLOCK_RATE_REFCLK;
    fwk_assert(ctx.dmc_refclk_ratio > 0);

    FWK_LOG_INFO("[DMC] Initializing DMC-400 at 0x%x", (uintptr_t)dmc);

    status = ddr_clk_init(id);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_phy_init(id);
    if (status != FWK_SUCCESS)
        return status;

    status = ddr_dmc_init(id);
    if (status != FWK_SUCCESS)
        return status;

    fwk_interrupt_global_disable();

    status = ddr_training(id);
    if (status != FWK_SUCCESS)
        return status;

    fwk_interrupt_global_enable();

    status = fwk_interrupt_set_isr(PHY_TRAINING_IRQ, ddr_phy_irq_handler);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    status = fwk_interrupt_clear_pending(PHY_TRAINING_IRQ);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    fwk_interrupt_enable(PHY_TRAINING_IRQ);

    /* Configure Integration Tests */
    dmc->INTEG_CFG = 0x00000000;
    dmc->INTEG_OUTPUTS = 0x00000000;

    FWK_LOG_INFO("[DMC] DDR Ready");

    /* Switch to READY */
    dmc->MEMC_CMD = DMC400_CMD_GO;

    return fwk_notification_subscribe(
        mod_pd_notification_id_power_state_pre_transition,
        element_config->pd_id,
        id);
}

static int juno_dmc400_process_notification(const struct fwk_event *event,
                                            struct fwk_event *resp_event)
{
    const struct mod_juno_dmc400_element_config *element_config;
    const struct mod_pd_power_state_transition_notification_params
        *pd_transition_params;
    const struct mod_pd_power_state_pre_transition_notification_params
        *pd_pre_transition_params;
    int status;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params;

    if (!fwk_module_is_valid_element_id(event->target_id))
        return FWK_E_PARAM;

    element_config = fwk_module_get_data(event->target_id);

    if (fwk_id_is_equal(event->id,
        mod_pd_notification_id_power_state_transition)) {
        pd_transition_params =
            (const struct mod_pd_power_state_transition_notification_params *)
                event->params;
        if (pd_transition_params->state != MOD_PD_STATE_ON)
            return FWK_SUCCESS;
        else
            return ddr_resume(element_config, event->target_id);
    } else if (fwk_id_is_equal(event->id,
        mod_pd_notification_id_power_state_pre_transition)) {
        pd_pre_transition_params =
        (const struct mod_pd_power_state_pre_transition_notification_params *)
            event->params;
        pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_event->params;

        if ((pd_pre_transition_params->target_state == MOD_PD_STATE_OFF) ||
            (pd_pre_transition_params->target_state ==
                MOD_SYSTEM_POWER_POWER_STATE_SLEEP0)) {
            status = ddr_suspend(element_config, event->target_id);

            pd_resp_params->status = status;

            return status;
        }
        else
            return FWK_SUCCESS;
    } else
        return FWK_E_HANDLER;
}

static int juno_dmc400_process_event(const struct fwk_event *event,
                                     struct fwk_event *resp)
{
    if (fwk_id_get_event_idx(event->id) >= JUNO_DMC400_EVENT_IDX_COUNT)
        return FWK_E_PARAM;

    return ddr_retraining(event->target_id);
}

struct fwk_module module_juno_dmc400 = {
    .name = "JUNO DMC-400",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 0,
    .event_count = JUNO_DMC400_EVENT_IDX_COUNT,
    .init = juno_dmc400_init,
    .element_init = juno_dmc400_element_init,
    .bind = juno_dmc400_bind,
    .start = juno_dmc400_start,
    .process_notification = juno_dmc400_process_notification,
    .process_event = juno_dmc400_process_event,
};
