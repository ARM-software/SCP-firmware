/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_I2C_DRIVER_H
#define INTERNAL_I2C_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#define I2C_POLLING_LIMIT (0x00100000U)

typedef enum {
    I2C_ERR_OK = 0,
    I2C_ERR_PARAM,
    I2C_ERR_BUSY,
    I2C_ERR_BER,
    I2C_ERR_BER_MC,
    I2C_ERR_POLLING,
    I2C_ERR_UNAVAILABLE
} I2C_ERR_t;

typedef enum {
    I2C_EN_EVF_NONE = 0,
    I2C_EN_EVF_FINISH,
    I2C_EN_EVF_BUSERROR,
    I2C_EN_EVF_MC_BUSERROR,
    I2C_EN_EVF_TIMEOUT,
    I2C_EN_EVF_NAK
} I2C_EN_EVF_t;

typedef enum { I2C_TYPE_F_I2C = 0, I2C_TYPE_F_I2C_SP1 } I2C_TYPE;

typedef union {
    uint8_t DATA;
    struct {
        /** B[0] READ/WRITE */
        uint32_t BITFIELD_READ : 1;
        /** B[7:1] TARGET ADDRESS */
        uint32_t BITFIELD_ADDR : 7;
    } bit;
} I2C_UN_SLVADDR_t;

typedef union {
    struct {
        uint8_t FSR_FS;
        uint8_t CSR_CS;
        uint8_t CCR_CS;
        uint8_t CCR_FM;
    } I2C_PARAM_F_I2C;
    struct {
        uint8_t NFR_NF;
        uint8_t NFR_NFH;
        uint8_t TLWR_TLW;
        uint8_t TLW2R_TLW;
        uint8_t THWR_THW;
        uint8_t THW2R_THW;
        uint8_t TBFR_TBF;
        uint8_t TBF2R_TBF;
        uint8_t TRSR_TRS;
        uint8_t TRS2R_TRS;
        uint8_t TSHR_TSH;
        uint8_t TSH2R_TSH;
        uint8_t TPSR_TPS;
        uint8_t TPS2R_TPS;
        uint8_t TLWRH_TLWH;
        uint8_t THWRH_THWH;
        uint8_t TRSRH_TRSH;
        uint8_t TSHRH_TSHH;
        uint8_t TPSRH_TPSH;
    } I2C_PARAM_F_I2C_SP1;
} I2C_PARAM_t;

typedef struct {
    char *BUFF;
    bool *ATTR;
    int SIZE;
    int LIMIT;
    int INDEX;
} I2C_ST_FIFO_t;

typedef struct {
    I2C_ST_FIFO_t CTRL;
    I2C_ST_FIFO_t DATA;
} I2C_ST_PACKET_t;

typedef struct {
    I2C_TYPE TYPE;
    uint32_t I2C_BASE_ADDR;
    bool USE_HS_MODE;
    bool CONTROLLER_CODE_FLAG;
    I2C_ST_PACKET_t PACKET;
    char CTRL_BUFF[0x08];
    bool CTRL_ATTR[0x08];
} I2C_ST_PACKET_INFO_t;

I2C_ST_PACKET_t *i2c_packet_initialize(I2C_ST_PACKET_t *packet);

I2C_ST_PACKET_t *i2c_packet_set_payload(
    I2C_ST_PACKET_t *packet,
    char *buffer,
    int size);

I2C_ST_PACKET_t *i2c_packet_set_control(
    I2C_ST_PACKET_t *packet,
    uint32_t address,
    bool read);

I2C_ST_PACKET_t *i2c_packet_set_address(
    I2C_ST_PACKET_t *packet,
    int address,
    int size);

I2C_ERR_t i2c_handler_polling(I2C_ST_PACKET_INFO_t *packet_info);

I2C_ERR_t i2c_initialize(
    I2C_ST_PACKET_INFO_t *packet_info,
    uint32_t reg_base,
    I2C_TYPE type,
    const I2C_PARAM_t *param);

I2C_ERR_t i2c_exec_transfer(I2C_ST_PACKET_INFO_t *packet_info);

void i2c_disable(I2C_ST_PACKET_INFO_t *packet);

void i2c_enable(I2C_ST_PACKET_INFO_t *packet_info);

#endif /* INTERNAL_I2C_DRIVER_H */
