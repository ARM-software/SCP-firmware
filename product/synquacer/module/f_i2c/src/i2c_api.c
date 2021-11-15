/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <i2c_api.h>

#include <internal/i2c_depend.h>
#include <internal/i2c_driver.h>

#include <stdbool.h>
#include <stdio.h>

#define FILE_GRP_ID DBG_DRV_I2C
#define CONTROLLER_CODE_VAL 0x04

static I2C_ERR_t i2c_api_recv_data_i(
    I2C_EN_CH_t ch,
    uint32_t target_address,
    uint32_t address,
    uint8_t *data,
    int length,
    int flgid);

static I2C_ERR_t i2c_api_send_data_i(
    I2C_EN_CH_t ch,
    uint32_t target_address,
    uint32_t address,
    const uint8_t *data,
    int length,
    int flgid);

I2C_ERR_t f_i2c_api_initialize(
    I2C_ST_PACKET_INFO_t *packet_info,
    uint32_t reg_base,
    I2C_TYPE type,
    const I2C_PARAM_t *param)
{
    I2C_ERR_t ercd = I2C_ERR_OK;
    ercd = i2c_initialize(packet_info, reg_base, type, param);
    return ercd;
}

I2C_ERR_t i2c_api_use_hs_mode(I2C_EN_CH_t ch, bool use_hsmode_flag)
{
    I2C_ST_PACKET_INFO_t *packet_info = i2c_get_channel_structure(ch);

    if (packet_info == NULL)
        return I2C_ERR_PARAM;

    if (packet_info->TYPE != I2C_TYPE_F_I2C_SP1)
        return I2C_ERR_UNAVAILABLE;

    packet_info->USE_HS_MODE = use_hsmode_flag;
    return I2C_ERR_OK;
}

static I2C_ERR_t i2c_api_recv_data_i(
    I2C_EN_CH_t ch,
    uint32_t target_address,
    uint32_t address,
    uint8_t *data,
    int length,
    int flgid)
{
    I2C_ERR_t ercd = I2C_ERR_OK;
    I2C_ST_PACKET_INFO_t *packet_info = i2c_get_channel_structure(ch);

    if (packet_info == NULL)
        return I2C_ERR_PARAM;

    i2c_enable(packet_info);

    i2c_packet_initialize(&packet_info->PACKET);

    /* Send controller code, only for F_I2C_SP1 */
    if (packet_info->TYPE == I2C_TYPE_F_I2C_SP1) {
        if (packet_info->USE_HS_MODE) {
            packet_info->CONTROLLER_CODE_FLAG = true;
            i2c_packet_set_control(
                &packet_info->PACKET, CONTROLLER_CODE_VAL, false);
        }
    }

    i2c_packet_set_control(&packet_info->PACKET, target_address, false);
    i2c_packet_set_address(&packet_info->PACKET, address, 1);
    i2c_packet_set_control(&packet_info->PACKET, target_address, true);
    i2c_packet_set_payload(&packet_info->PACKET, (char *)data, length);

    ercd = i2c_exec_transfer(packet_info);
    if ((flgid == 0) && (ercd == I2C_ERR_OK))
        ercd = i2c_handler_polling(packet_info);

    i2c_disable(packet_info);

    return ercd;
}

I2C_ERR_t f_i2c_api_recv_data(
    I2C_EN_CH_t ch,
    uint32_t target_address,
    uint32_t address,
    uint8_t *data,
    int length)
{
    I2C_ERR_t ercd = I2C_ERR_OK;

    if ((target_address <= 0x03) || (target_address >= 0x78))
        return I2C_ERR_PARAM;

    if (data == NULL)
        return I2C_ERR_PARAM;

    if (length <= 0)
        return I2C_ERR_PARAM;

    ercd = i2c_api_recv_data_i(
        ch, target_address, address, data, length, 0x00000000);

    return ercd;
}

static I2C_ERR_t i2c_api_send_data_i(
    I2C_EN_CH_t ch,
    uint32_t target_address,
    uint32_t address,
    const uint8_t *data,
    int length,
    int flgid)
{
    I2C_ERR_t ercd = I2C_ERR_OK;
    I2C_ST_PACKET_INFO_t *packet_info = i2c_get_channel_structure(ch);

    if (packet_info == NULL)
        return I2C_ERR_PARAM;

    i2c_enable(packet_info);

    i2c_packet_initialize(&packet_info->PACKET);

    /* Send controller code, only for F_I2C_SP1 */
    if (packet_info->TYPE == I2C_TYPE_F_I2C_SP1) {
        if (packet_info->USE_HS_MODE) {
            packet_info->CONTROLLER_CODE_FLAG = true;
            i2c_packet_set_control(
                &packet_info->PACKET, CONTROLLER_CODE_VAL, false);
        }
    }

    i2c_packet_set_control(&packet_info->PACKET, target_address, false);
    i2c_packet_set_address(&packet_info->PACKET, address, 1);
    i2c_packet_set_payload(&packet_info->PACKET, (char *)data, length);

    ercd = i2c_exec_transfer(packet_info);
    if ((flgid == 0) && (ercd == I2C_ERR_OK))
        ercd = i2c_handler_polling(packet_info);

    i2c_disable(packet_info);

    return ercd;
}

I2C_ERR_t f_i2c_api_send_data(
    I2C_EN_CH_t ch,
    uint32_t target_address,
    uint32_t address,
    const uint8_t *data,
    int length)
{
    I2C_ERR_t ercd = I2C_ERR_OK;

    if ((target_address == 0x01) || (target_address == 0x02) ||
        (target_address == 0x03) || (target_address >= 0x78)) {
        return I2C_ERR_PARAM;
    }

    if (length < 0)
        return I2C_ERR_PARAM;

    ercd = i2c_api_send_data_i(
        ch, target_address, address, data, length, 0x00000000);

    return ercd;
}
