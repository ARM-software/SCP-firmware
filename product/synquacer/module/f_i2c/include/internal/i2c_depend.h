/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_I2C_DEPEND_H
#define INTERNAL_I2C_DEPEND_H

#include <internal/i2c_driver.h>

#include <stdint.h>

typedef enum { I2C_EN_CH0, I2C_EN_CH1, I2C_EN_CH2 } I2C_EN_CH_t;

I2C_ST_PACKET_INFO_t *i2c_get_channel_structure(I2C_EN_CH_t ch);

void i2c_construction(void);
void i2c_destruction(void);
I2C_ERR_t f_i2c_api_initialize(
    I2C_ST_PACKET_INFO_t *packet_info,
    uint32_t reg_base,
    I2C_TYPE type,
    const I2C_PARAM_t *param);

#endif /* INTERNAL_I2C_DEPEND_H */
