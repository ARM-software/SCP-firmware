/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef I2C_API_H
#define I2C_API_H

#include <internal/i2c_depend.h>
#include <internal/i2c_driver.h>

#include <stdint.h>

#define I2C_MP_START_ADDR 0x50

I2C_ERR_t f_i2c_api_recv_data(
    I2C_EN_CH_t ch,
    uint32_t slave_address,
    uint32_t address,
    uint8_t *data,
    int length);

I2C_ERR_t f_i2c_api_send_data(
    I2C_EN_CH_t ch,
    uint32_t slave_address,
    uint32_t address,
    const uint8_t *data,
    int length);

#endif /* I2C_API_H */
