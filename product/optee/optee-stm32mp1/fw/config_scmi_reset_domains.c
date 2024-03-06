/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dt-bindings/reset/stm32mp1-resets.h>
#include <scmi_agents.h>
#include <stm32_util.h>
#include <util.h>

#include <mod_optee_reset.h>
#include <mod_reset_domain.h>
#include <mod_scmi_reset_domain.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <assert.h>

/*
 * Indices of reset domain elements exposed through a SCMI agent.
 * As all exposed SCMI reset domains relate to a single backend dirver
 * these indices are used as indices for fwk elements for modules
 * RESET_DOMAIN and STM32_RESET.
 */
enum resetd_elt_idx {
    /* Reset domain exposed to agent SCMI */
    RESETD_IDX_SCMI_SPI6,
    RESETD_IDX_SCMI_I2C4,
    RESETD_IDX_SCMI_I2C6,
    RESETD_IDX_SCMI_USART1,
    RESETD_IDX_SCMI_STGEN,
    RESETD_IDX_SCMI_GPIOZ,
    RESETD_IDX_SCMI_CRYP1,
    RESETD_IDX_SCMI_HASH1,
    RESETD_IDX_SCMI_RNG1,
    RESETD_IDX_SCMI_MDMA,
    RESETD_IDX_SCMI_MCU,
    RESETD_IDX_SCMI_MCU_HOLD_BOOT,
    RESETD_IDX_COUNT
};

struct mod_stm32_reset_dev_config {
    const char *name;
    unsigned long rcc_rst_id;
};

/*
 * stm32_resetd_cfg - Common configuration for exposed SCMI reset domains
 *
 * Domain names defined here are used for all RESET_DOMAIN and STM32_RESET
 * fwk elements names.
 */
#define STM32_RESET_CFG(_idx, _rcc_rst_id, _name) \
    [(_idx)] = { .rcc_rst_id = (_rcc_rst_id), .name = (_name) }

static const struct mod_stm32_reset_dev_config stm32_resetd_cfg[] = {
    STM32_RESET_CFG(RESETD_IDX_SCMI_SPI6, SPI6_R, "spi6"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_I2C4, I2C4_R, "i2c4"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_I2C6, I2C6_R, "i2c6"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_USART1, USART1_R, "usart1"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_STGEN, STGEN_R, "stgen"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_GPIOZ, GPIOZ_R, "gpioz"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_CRYP1, CRYP1_R, "cryp1"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_HASH1, HASH1_R, "hash1"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_RNG1, RNG1_R, "rng1"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_MDMA, MDMA_R, "mdma"),
    STM32_RESET_CFG(RESETD_IDX_SCMI_MCU, MCU_R, "mcu"),
    STM32_RESET_CFG(
        RESETD_IDX_SCMI_MCU_HOLD_BOOT,
        MCU_HOLD_BOOT_R,
        "mcu-hold-boot"),
};

/*
 * Bindgins between SCMI domain_id value and reset domain module element in fwk
 */
#define SCMI_RESETD_ELT_ID(_idx) \
    { \
        .element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, (_idx)) \
    }

static const struct mod_scmi_reset_domain_device scmi_resetd_device[] = {
    [RST_SCMI_SPI6] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_SPI6),
    [RST_SCMI_I2C4] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_I2C4),
    [RST_SCMI_I2C6] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_I2C6),
    [RST_SCMI_USART1] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_USART1),
    [RST_SCMI_STGEN] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_STGEN),
    [RST_SCMI_GPIOZ] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_GPIOZ),
    [RST_SCMI_CRYP1] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_CRYP1),
    [RST_SCMI_HASH1] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_HASH1),
    [RST_SCMI_RNG1] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_RNG1),
    [RST_SCMI_MDMA] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_MDMA),
    [RST_SCMI_MCU] = SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_MCU),
    [RST_SCMI_MCU_HOLD_BOOT] =
        SCMI_RESETD_ELT_ID(RESETD_IDX_SCMI_MCU_HOLD_BOOT),
};

/* Agents andreset domains references */
static const struct mod_scmi_reset_domain_agent resetd_agent_table[SCMI_AGENT_ID_COUNT] = {
    [SCMI_AGENT_ID_NSEC0] = {
        .device_table = (void *)scmi_resetd_device,
        .agent_domain_count = FWK_ARRAY_SIZE(scmi_resetd_device),
    },
};

/* Exported configuration data for module SCMI_RESET_DOMAIN */
struct fwk_module_config config_scmi_reset_domain = {
    .data = &((struct mod_scmi_reset_domain_config){
        .agent_table = resetd_agent_table,
        .agent_count = FWK_ARRAY_SIZE(resetd_agent_table),
    }),
};

/*
 * Reset controller backend driver configuration
 * STM32_RESET element index is the related RESET_DOMAIN element index.
 */
#define RESETD_DATA(_idx) \
    ((struct mod_reset_domain_dev_config){ \
        .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_RESET, (_idx)), \
        .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_RESET, 0), \
        .modes = MOD_RESET_DOMAIN_AUTO_RESET | \
            MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT | \
            MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT, \
        .capabilities = 0, /* No notif, no async */ \
    })

#define RESETD_ELT(_idx) \
    [(_idx)] = { \
        .name = stm32_resetd_cfg[(_idx)].name, \
        .data = &RESETD_DATA((_idx)), \
    }

/* Element names are the reset domain names exposed by the SCMI service */
static const struct fwk_element resetd_elt[] = {
    /* Reset domains exposed to agent SCMI */
    RESETD_ELT(RESETD_IDX_SCMI_SPI6),
    RESETD_ELT(RESETD_IDX_SCMI_I2C4),
    RESETD_ELT(RESETD_IDX_SCMI_I2C6),
    RESETD_ELT(RESETD_IDX_SCMI_USART1),
    RESETD_ELT(RESETD_IDX_SCMI_STGEN),
    RESETD_ELT(RESETD_IDX_SCMI_GPIOZ),
    RESETD_ELT(RESETD_IDX_SCMI_CRYP1),
    RESETD_ELT(RESETD_IDX_SCMI_HASH1),
    RESETD_ELT(RESETD_IDX_SCMI_RNG1),
    RESETD_ELT(RESETD_IDX_SCMI_MDMA),
    RESETD_ELT(RESETD_IDX_SCMI_MCU),
    RESETD_ELT(RESETD_IDX_SCMI_MCU_HOLD_BOOT),
    /* Termination entry */
    [RESETD_IDX_COUNT] = { 0 }
};

static_assert(
    FWK_ARRAY_SIZE(resetd_elt) == RESETD_IDX_COUNT + 1,
    "Invalid range for RESET_DOMAIN and STM32_RESET indices");

/* Exported configuration data for module VOLTAGE_DOMAIN */
const struct fwk_module_config config_reset_domain = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(resetd_elt),
};

/*
 * Configuration for module OPTEE_RESET
 */
#define RESET_COUNT FWK_ARRAY_SIZE(stm32_resetd_cfg)
static struct mod_optee_reset_dev_config optee_reset_cfg[RESET_COUNT];

#define OPTEE_RESET_ELT(_idx) \
    [(_idx)] = { \
        .name = stm32_resetd_cfg[(_idx)].name, \
        .data = &optee_reset_cfg[(_idx)], \
    }

static const struct fwk_element optee_reset_elt[] = {
    /* Reset domaines exposed to agent SCMI */
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_SPI6),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_I2C4),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_I2C6),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_USART1),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_STGEN),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_GPIOZ),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_CRYP1),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_HASH1),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_RNG1),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_MDMA),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_MCU),
    OPTEE_RESET_ELT(RESETD_IDX_SCMI_MCU_HOLD_BOOT),
    /* Termination entry */
    [RESETD_IDX_COUNT] = { 0 }
};

static_assert(
    FWK_ARRAY_SIZE(optee_reset_elt) == RESETD_IDX_COUNT + 1,
    "Invalid range for RESET and OPTEE_RESET indices");

static const struct fwk_element *optee_reset_get_elt_table(fwk_id_t module_id)
{
    size_t n;

    for (n = 0; n < FWK_ARRAY_SIZE(optee_reset_cfg); n++) {
        optee_reset_cfg[n].rstctrl =
            stm32mp_rcc_reset_id_to_rstctrl(stm32_resetd_cfg[n].rcc_rst_id);
    }

    return optee_reset_elt;
}

struct fwk_module_config config_optee_reset = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(optee_reset_get_elt_table),
};
