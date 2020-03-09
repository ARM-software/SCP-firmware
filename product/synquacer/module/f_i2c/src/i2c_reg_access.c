/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/i2c_driver.h>
#include <internal/i2c_reg.h>
#include <internal/i2c_reg_access.h>

#include <stdint.h>

static inline void i2c_write_reg(
    uint32_t base_addr,
    uint32_t reg_addr,
    uint8_t value)
{
    *((volatile uint8_t *)(base_addr + reg_addr)) = value;
}

static inline uint8_t i2c_read_reg(uint32_t base_addr, uint32_t reg_addr)
{
    return *((volatile uint8_t *)(base_addr + reg_addr));
}

/* F_I2C Register Write */
void f_i2c_write_BSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_BSR, value);
    return;
}
void f_i2c_write_BCR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_BCR, value);
    return;
}
void f_i2c_write_CCR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_CCR, value);
    return;
}
void f_i2c_write_ADR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_ADR, value);
    return;
}
void f_i2c_write_DAR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_DAR, value);
    return;
}
void f_i2c_write_CSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_CSR, value);
    return;
}
void f_i2c_write_FSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_FSR, value);
    return;
}
void f_i2c_write_BC2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_BC2R, value);
    return;
}
void f_i2c_write_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    while (1)
        ;
}

/* F_I2C Register Read */
uint8_t f_i2c_read_BSR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_BSR);
}
uint8_t f_i2c_read_BCR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_BCR);
}
uint8_t f_i2c_read_CCR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_CCR);
}
uint8_t f_i2c_read_ADR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_ADR);
}
uint8_t f_i2c_read_DAR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_DAR);
}
uint8_t f_i2c_read_CSR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_CSR);
}
uint8_t f_i2c_read_FSR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_FSR);
}
uint8_t f_i2c_read_BC2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_REG_ADDR_BC2R);
}
uint8_t f_i2c_read_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info)
{
    while (1)
        ;
}

/* F_I2C_SP1 Register Write */
void f_i2c_sp1_write_BSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BSR, value);
    return;
}
void f_i2c_sp1_write_BS2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BS2R, value);
    return;
}
void f_i2c_sp1_write_BCR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BCR, value);
    return;
}
void f_i2c_sp1_write_BC2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BC2R, value);
    return;
}
void f_i2c_sp1_write_ADR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_ADR, value);
    return;
}
void f_i2c_sp1_write_DAR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_DAR, value);
    return;
}
void f_i2c_sp1_write_NFR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_NFR, value);
    return;
}
void f_i2c_sp1_write_TLWR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TLWR, value);
    return;
}
void f_i2c_sp1_write_TLW2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TLW2R, value);
    return;
}
void f_i2c_sp1_write_THWR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_THWR, value);
    return;
}
void f_i2c_sp1_write_THW2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_THW2R, value);
    return;
}
void f_i2c_sp1_write_TBFR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TBFR, value);
    return;
}
void f_i2c_sp1_write_TBF2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TBF2R, value);
    return;
}
void f_i2c_sp1_write_TRSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TRSR, value);
    return;
}
void f_i2c_sp1_write_TRS2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TRS2R, value);
    return;
}
void f_i2c_sp1_write_TSHR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TSHR, value);
    return;
}
void f_i2c_sp1_write_TSH2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TSH2R, value);
    return;
}
void f_i2c_sp1_write_TPSR(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TPSR, value);
    return;
}
void f_i2c_sp1_write_TPS2R(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TPS2R, value);
    return;
}
void f_i2c_sp1_write_TLWRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TLWRH, value);
    return;
}
void f_i2c_sp1_write_THWRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_THWRH, value);
    return;
}
void f_i2c_sp1_write_TRSRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TRSRH, value);
    return;
}
void f_i2c_sp1_write_TSHRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TSHRH, value);
    return;
}
void f_i2c_sp1_write_TPSRH(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    i2c_write_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TPSRH, value);
    return;
}
void f_i2c_sp1_write_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info, uint8_t value)
{
    while (1)
        ;
}

/* F_I2C_SP1 Register Read */
uint8_t f_i2c_sp1_read_BSR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BSR);
}
uint8_t f_i2c_sp1_read_BS2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BS2R);
}
uint8_t f_i2c_sp1_read_BCR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BCR);
}
uint8_t f_i2c_sp1_read_BC2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_BC2R);
}
uint8_t f_i2c_sp1_read_ADR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_ADR);
}
uint8_t f_i2c_sp1_read_DAR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_DAR);
}
uint8_t f_i2c_sp1_read_NFR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_NFR);
}
uint8_t f_i2c_sp1_read_TLWR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TLWR);
}
uint8_t f_i2c_sp1_read_TLW2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TLW2R);
}
uint8_t f_i2c_sp1_read_THWR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_THWR);
}
uint8_t f_i2c_sp1_read_THW2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_THW2R);
}
uint8_t f_i2c_sp1_read_TBFR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TBFR);
}
uint8_t f_i2c_sp1_read_TBF2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TBF2R);
}
uint8_t f_i2c_sp1_read_TRSR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TRSR);
}
uint8_t f_i2c_sp1_read_TRS2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TRS2R);
}
uint8_t f_i2c_sp1_read_TSHR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TSHR);
}
uint8_t f_i2c_sp1_read_TSH2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TSH2R);
}
uint8_t f_i2c_sp1_read_TPSR(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TPSR);
}
uint8_t f_i2c_sp1_read_TPS2R(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TPS2R);
}
uint8_t f_i2c_sp1_read_TLWRH(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TLWRH);
}
uint8_t f_i2c_sp1_read_THWRH(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_THWRH);
}
uint8_t f_i2c_sp1_read_TRSRH(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TRSRH);
}
uint8_t f_i2c_sp1_read_TSHRH(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TSHRH);
}
uint8_t f_i2c_sp1_read_TPSRH(I2C_ST_PACKET_INFO_t *packet_info)
{
    return i2c_read_reg(packet_info->I2C_BASE_ADDR, I2C_SP1_REG_ADDR_TPSRH);
}
uint8_t f_i2c_sp1_read_UNDEFINED(I2C_ST_PACKET_INFO_t *packet_info)
{
    while (1)
        ;
}
