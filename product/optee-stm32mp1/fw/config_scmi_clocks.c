/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dt-bindings/clock/stm32mp1-clks.h>
#include <scmi_agents.h>
#include <stm32_util.h>
#include <util.h>

#include <mod_clock.h>
#include <mod_optee_clock.h>
#include <mod_scmi_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Indices of clock elements exposed through a SCMI agent.
 * As all exposed SCMI clocks relate to a single backend dirver
 * these indices are used as indices for fwk elements for modules
 * CLOCK and STM32_CLOCK. Note these are not the clock ID values
 * exposed through SCMI.
 */
enum clock_elt_idx {
    /* Clocks exposed to agent SCMI */
    CLK_IDX_SCMI_HSE,
    CLK_IDX_SCMI_HSI,
    CLK_IDX_SCMI_CSI,
    CLK_IDX_SCMI_LSE,
    CLK_IDX_SCMI_LSI,
    CLK_IDX_SCMI_PLL2_Q,
    CLK_IDX_SCMI_PLL2_R,
    CLK_IDX_SCMI_MPU,
    CLK_IDX_SCMI_AXI,
    CLK_IDX_SCMI_BSEC,
    CLK_IDX_SCMI_CRYP1,
    CLK_IDX_SCMI_GPIOZ,
    CLK_IDX_SCMI_HASH1,
    CLK_IDX_SCMI_I2C4,
    CLK_IDX_SCMI_I2C6,
    CLK_IDX_SCMI_IWDG1,
    CLK_IDX_SCMI_RNG1,
    CLK_IDX_SCMI_RTC,
    CLK_IDX_SCMI_RTCAPB,
    CLK_IDX_SCMI_SPI6,
    CLK_IDX_SCMI_USART1,
    /* Count indices */
    CLK_IDX_COUNT
};

struct mod_stm32_clock_dev_config {
    const char *name;
    unsigned long rcc_clk_id;
    bool default_enabled;
};

/*
 * stm32_clock_cfg - Common configuration for exposed SCMI clocks
 *
 * Clock name defined here is used for all CLOCK and STM32_CLOCK
 * fwk elements names.
 */
#define STM32_CLOCK_CFG(_idx, _rcc_clk_id, _name, _default_enabled) \
    [(_idx)] = { \
        .rcc_clk_id = (_rcc_clk_id), \
        .name = (_name), \
        .default_enabled = (_default_enabled), \
    }

static const struct mod_stm32_clock_dev_config stm32_clock_cfg[] = {
    STM32_CLOCK_CFG(CLK_IDX_SCMI_HSE, CK_HSE, "ck_hse", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_HSI, CK_HSI, "ck_hsi", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_CSI, CK_CSI, "ck_csi", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_LSE, CK_LSE, "ck_lse", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_LSI, CK_LSI, "ck_lsi", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_PLL2_Q, PLL2_Q, "pll2_q", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_PLL2_R, PLL2_R, "pll2_r", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_MPU, CK_MCU, "ck_mpu", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_AXI, CK_AXI, "ck_axi", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_BSEC, BSEC, "bsec", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_CRYP1, CRYP1, "cryp1", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_GPIOZ, GPIOZ, "gpioz", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_HASH1, HASH1, "hash1", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_I2C4, I2C4_K, "i2c4_k", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_I2C6, I2C6_K, "i2c6_k", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_IWDG1, IWDG1, "iwdg1", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_RNG1, RNG1_K, "rng1_k", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_RTC, RTC, "ck_rtc", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_RTCAPB, RTCAPB, "rtcapb", true),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_SPI6, SPI6_K, "spi6_k", false),
    STM32_CLOCK_CFG(CLK_IDX_SCMI_USART1, USART1_K, "usart1_k", false),
};

/*
 * Bindgins between SCMI clock_id value and clock module element in fwk
 */
#define SCMI_CLOCK_ELT_ID(_idx) \
    { \
        .element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, (_idx)) \
    }

static struct mod_scmi_clock_device scmi_clock_device[] = {
    [CK_SCMI_HSE] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_HSE),
    [CK_SCMI_HSI] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_HSI),
    [CK_SCMI_CSI] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_CSI),
    [CK_SCMI_LSE] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_LSE),
    [CK_SCMI_LSI] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_LSI),
    [CK_SCMI_PLL2_Q] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_PLL2_Q),
    [CK_SCMI_PLL2_R] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_PLL2_R),
    [CK_SCMI_MPU] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_MPU),
    [CK_SCMI_AXI] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_AXI),
    [CK_SCMI_BSEC] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_BSEC),
    [CK_SCMI_CRYP1] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_CRYP1),
    [CK_SCMI_GPIOZ] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_GPIOZ),
    [CK_SCMI_HASH1] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_HASH1),
    [CK_SCMI_I2C4] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_I2C4),
    [CK_SCMI_I2C6] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_I2C6),
    [CK_SCMI_IWDG1] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_IWDG1),
    [CK_SCMI_RNG1] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_RNG1),
    [CK_SCMI_RTC] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_RTC),
    [CK_SCMI_RTCAPB] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_RTCAPB),
    [CK_SCMI_SPI6] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_SPI6),
    [CK_SCMI_USART1] = SCMI_CLOCK_ELT_ID(CLK_IDX_SCMI_USART1),
};

/* Agents and clocks references */
static const struct mod_scmi_clock_agent clk_agent_tbl[SCMI_AGENT_ID_COUNT] = {
    [SCMI_AGENT_ID_NSEC0] = {
        .device_table = (void *)scmi_clock_device,
        .device_count = FWK_ARRAY_SIZE(scmi_clock_device),
    },
};

/* Exported configuration data for module SCMI_CLOCK */
struct fwk_module_config config_scmi_clock = {
    .data = &((struct mod_scmi_clock_config){
        .agent_table = clk_agent_tbl,
        .agent_count = FWK_ARRAY_SIZE(clk_agent_tbl),
    }),
};

/*
 * Clock backend driver configuration
 * STM32_CLOCK element index is the related CLOCK element index.
 */
#define CLOCK_DATA(_idx) \
    ((struct mod_clock_dev_config){ \
        .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_CLOCK, (_idx)), \
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_CLOCK, 0), \
    })

#define CLOCK_ELT(_idx) \
    [(_idx)] = { \
        .name = stm32_clock_cfg[(_idx)].name, \
        .data = &CLOCK_DATA((_idx)), \
    }

/* Element names are the clock names exposed by the SCMI service */
static struct fwk_element clock_elt[] = {
    /* Clocks exposed to agent SCMI */
    CLOCK_ELT(CLK_IDX_SCMI_HSE),
    CLOCK_ELT(CLK_IDX_SCMI_HSI),
    CLOCK_ELT(CLK_IDX_SCMI_CSI),
    CLOCK_ELT(CLK_IDX_SCMI_LSE),
    CLOCK_ELT(CLK_IDX_SCMI_LSI),
    CLOCK_ELT(CLK_IDX_SCMI_PLL2_Q),
    CLOCK_ELT(CLK_IDX_SCMI_PLL2_R),
    CLOCK_ELT(CLK_IDX_SCMI_MPU),
    CLOCK_ELT(CLK_IDX_SCMI_AXI),
    CLOCK_ELT(CLK_IDX_SCMI_BSEC),
    CLOCK_ELT(CLK_IDX_SCMI_CRYP1),
    CLOCK_ELT(CLK_IDX_SCMI_GPIOZ),
    CLOCK_ELT(CLK_IDX_SCMI_HASH1),
    CLOCK_ELT(CLK_IDX_SCMI_I2C4),
    CLOCK_ELT(CLK_IDX_SCMI_I2C6),
    CLOCK_ELT(CLK_IDX_SCMI_IWDG1),
    CLOCK_ELT(CLK_IDX_SCMI_RNG1),
    CLOCK_ELT(CLK_IDX_SCMI_RTC),
    CLOCK_ELT(CLK_IDX_SCMI_RTCAPB),
    CLOCK_ELT(CLK_IDX_SCMI_SPI6),
    CLOCK_ELT(CLK_IDX_SCMI_USART1),
    /* Termination entry */
    [CLK_IDX_COUNT] = { 0 }
};

static_assert(
    FWK_ARRAY_SIZE(clock_elt) == CLK_IDX_COUNT + 1,
    "Invalid range for CLOCK and STM32_CLOCK indices");

/* Exported configuration data for module VOLTAGE_DOMAIN */
const struct fwk_module_config config_clock = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(clock_elt),
};

#define CLOCK_COUNT FWK_ARRAY_SIZE(stm32_clock_cfg)
static struct mod_optee_clock_config optee_clock_cfg[CLOCK_COUNT];

#define OPTEE_CLOCK_ELT(_idx) \
    [(_idx)] = { \
        .name = stm32_clock_cfg[(_idx)].name, \
        .data = &optee_clock_cfg[(_idx)], \
    }

static const struct fwk_element optee_clock_elt[] = {
    /* Clocks exposed to agent SCMI */
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_HSE),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_HSI),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_CSI),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_LSE),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_LSI),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_PLL2_Q),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_PLL2_R),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_MPU),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_AXI),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_BSEC),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_CRYP1),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_GPIOZ),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_HASH1),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_I2C4),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_I2C6),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_IWDG1),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_RNG1),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_RTC),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_RTCAPB),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_SPI6),
    OPTEE_CLOCK_ELT(CLK_IDX_SCMI_USART1),
    /* Termination entry */
    [CLK_IDX_COUNT] = { 0 }
};

static_assert(
    FWK_ARRAY_SIZE(optee_clock_elt) == CLK_IDX_COUNT + 1,
    "Invalid range for CLOCK and STM32_CLOCK indices");

static const struct fwk_element *optee_clock_get_elt_table(fwk_id_t module_id)
{
    size_t n;

    for (n = 0; n < FWK_ARRAY_SIZE(optee_clock_cfg); n++) {
        optee_clock_cfg[n].clk =
            stm32mp_rcc_clock_id_to_clk(stm32_clock_cfg[n].rcc_clk_id);
        optee_clock_cfg[n].default_enabled = stm32_clock_cfg[n].default_enabled;
    }

    return optee_clock_elt;
}

struct fwk_module_config config_optee_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(optee_clock_get_elt_table),
};
