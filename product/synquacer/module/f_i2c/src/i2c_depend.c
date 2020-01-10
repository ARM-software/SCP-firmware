/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <internal/i2c_depend.h>
#include <internal/i2c_driver.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>
#include <fwk_macros.h>

#include <stdio.h>

typedef unsigned int ADDR_T;

#define SCB_BUILDTIME_ASSERT(x)
#define FILE_GRP_ID DBG_DRV_I2C
#define SCB_Error(x)

#if CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 0)
static I2C_ST_PACKET_INFO_t m_stI2C0;
#endif /* CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 0) */

#if CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 1)
static I2C_ST_PACKET_INFO_t m_stI2C1;
#endif /* CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 1) */

#if CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 2)
static I2C_ST_PACKET_INFO_t m_stI2C2;
#endif /* CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 2) */

static const ADDR_T i2c_base_addrs[] = CONFIG_SOC_I2C_BASE_ADDRS;
static const I2C_TYPE i2c_types[] = CONFIG_SOC_I2C_TYPES;
static const I2C_EN_CH_t i2c_channels[] = CONFIG_SOC_I2C_CHANNELS;
static const I2C_PARAM_t i2c_params[] = CONFIG_SCB_I2C_PARAMS;

I2C_ST_PACKET_INFO_t *i2c_get_channel_structure(I2C_EN_CH_t ch)
{
    I2C_ST_PACKET_INFO_t *result = NULL;

    switch (ch) {
#if CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 0)
    case I2C_EN_CH0:
        result = &m_stI2C0;
        break;
#endif /* CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 0) */

#if CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 1)
    case I2C_EN_CH1:
        result = &m_stI2C1;
        break;
#endif /* CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 1) */

#if CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 2)
    case I2C_EN_CH2:
        result = &m_stI2C2;
        break;
#endif /* CONFIG_SOC_I2C_ENABLE_BITMAP & (1 << 2) */

    default:
        result = NULL;
        break;
    }

    return result;
}

void i2c_construction(void)
{
    I2C_ERR_t i2c_err;
    size_t i;

    /* Check config consistency */
    SCB_BUILDTIME_ASSERT(
        FWK_ARRAY_SIZE(i2c_types) == FWK_ARRAY_SIZE(i2c_base_addrs));
    SCB_BUILDTIME_ASSERT(
        FWK_ARRAY_SIZE(i2c_channels) == FWK_ARRAY_SIZE(i2c_base_addrs));
    SCB_BUILDTIME_ASSERT(
        FWK_ARRAY_SIZE(i2c_params) == FWK_ARRAY_SIZE(i2c_base_addrs));

    for (i = 0; i < FWK_ARRAY_SIZE(i2c_params); i++) {
        i2c_err = f_i2c_api_initialize(
            i2c_get_channel_structure(i2c_channels[i]),
            i2c_base_addrs[i],
            i2c_types[i],
            &i2c_params[i]);
        if (i2c_err != I2C_ERR_OK) {
            SCB_Error(i2c_err);
            FWK_LOG_ERR("[I2C] I2C ch#%d  initialize error.", i);
        }
    }
}

void i2c_destruction(void)
{
}
