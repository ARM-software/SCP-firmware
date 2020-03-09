/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_I2C_REG_ACCESS_H
#define INTERNAL_I2C_REG_ACCESS_H

#include <internal/i2c_driver.h>

#include <stdint.h>

/* F_I2C Register Write */
void f_i2c_write_BSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_BCR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_CCR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_ADR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_DAR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_CSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_FSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_BC2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_write_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);

/* F_I2C Register Read */
uint8_t f_i2c_read_BSR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_BCR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_CCR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_ADR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_DAR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_CSR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_FSR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_BC2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_read_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info);

/* F_I2C_SP1 Register Write */
void f_i2c_sp1_write_BSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_BS2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_BCR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_BC2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_ADR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_DAR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_NFR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TLWR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TLW2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_THWR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_THW2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TBFR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TBF2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TRSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TRS2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TSHR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TSH2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TPSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TPS2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TLWRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_THWRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TRSRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TSHRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_TPSRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
void f_i2c_sp1_write_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info,
                               uint8_t value);

/* F_I2C_SP1 Register Read */
uint8_t f_i2c_sp1_read_BSR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_BS2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_BCR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_BC2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_ADR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_DAR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_NFR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TLWR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TLW2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_THWR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_THW2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TBFR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TBF2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TRSR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TRS2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TSHR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TSH2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TPSR(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TPS2R(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TLWRH(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_THWRH(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TRSRH(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TSHRH(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_TPSRH(I2C_ST_PACKET_INFO_t *packet_info);
uint8_t f_i2c_sp1_read_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info);

struct I2C_REG_FUNC_TABLE {
    void (*set_BSR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_BS2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_BCR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_BC2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_ADR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_DAR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_NFR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TLWR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TLW2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_THWR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_THW2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TBFR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TBF2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TRSR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TRS2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TSHR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TSH2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TPSR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TPS2R)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TLWRH)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_THWRH)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TRSRH)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TSHRH)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_TPSRH)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_CCR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_CSR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);
    void (*set_FSR)(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value);

    uint8_t (*get_BSR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_BS2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_BCR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_BC2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_ADR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_DAR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_NFR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TLWR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TLW2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_THWR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_THW2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TBFR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TBF2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TRSR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TRS2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TSHR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TSH2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TPSR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TPS2R)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TLWRH)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_THWRH)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TRSRH)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TSHRH)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_TPSRH)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_CCR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_CSR)(I2C_ST_PACKET_INFO_t *packet_info);
    uint8_t (*get_FSR)(I2C_ST_PACKET_INFO_t *packet_info);
};

#endif /* INTERNAL_I2C_REG_ACCESS_H */
