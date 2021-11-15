/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_I2C_REG_H
#define INTERNAL_I2C_REG_H

#include <stdint.h>

typedef union {
    uint8_t DATA;
    struct {
        uint32_t FBT : 1; /* B00      First Byte Transfer */
        uint32_t reserved1 : 2; /* B01-02   Reserved */
        uint32_t TRX : 1; /* B03      Transfer/Receive */
        uint32_t LRB : 1; /* B04      LAST Received Bit */
        uint32_t reserved2 : 1; /* B05      Reserved */
        uint32_t RSC : 1; /* B06      Repeated Start Condition */
        uint32_t BB : 1; /* B07      Bus busy */
    } bit_COMMON;
    struct {
        uint32_t reserved1 : 1; /* B00      Reserved */
        uint32_t GCA : 1; /* B01      General Call Address */
        uint32_t AAS : 1; /* B02      Address As Target */
        uint32_t reserved2 : 2; /* B03-04   Reserved */
        uint32_t AL : 1; /* B05      Arbitration Lost */
        uint32_t reserved3 : 2; /* B06-07   Reserved */
    } bit_F_I2C;
} I2C_UN_BSR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t reserved1 : 6; /* B00-05   Reserved */
        uint32_t HS : 1; /* B06      Hi Speed Mode */
        uint32_t MAS : 1; /* B07      Controller Code */
    } bit_F_I2C_SP1;
} I2C_UN_BS2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t INT : 1; /* B00      INTerrupt */
        uint32_t INTE : 1; /* B01      INTerrupt Enable */
        uint32_t reserved1 : 1; /* B02      Reserved */
        uint32_t ACK : 1; /* B03      Acknowledge Enable */
        uint32_t MSS : 1; /* B04      Controller Target Select 1:Controller */
        uint32_t SCC : 1; /* B05      Start Condition Continue */
        uint32_t BEIE : 1; /* B06      Bus Error Interrupt Enable */
        uint32_t BER : 1; /* B07      Bus Error */
    } bit_COMMON;
    struct {
        uint32_t reserved1 : 2; /* B00-01   Reserved */
        uint32_t GCAA : 1; /* B02      General Call Address Acknowledge */
        uint32_t reserved2 : 5; /* B03-07   Reserved */
    } bit_F_I2C;
} I2C_UN_BCR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t SCLL : 1; /* B00      SCL Low drive */
        uint32_t SDAL : 1; /* B01      SDA Low drive */
        uint32_t reserved1 : 2; /* B02-B03  Reserved */
        uint32_t SCLS : 1; /* B04      SCL Status(Read Only) */
        uint32_t SDAS : 1; /* B05      SDA Status(Read Only) */
        uint32_t reserved2 : 2; /* B06-B07  Reserved */
    } bit_COMMON;
    struct {
        uint32_t reserved1 : 7; /* B00-07   Reserved */
        uint32_t EN : 1; /* B07      Enable */
    } bit_F_I2C_SP1;
} I2C_UN_BC2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t A : 7; /* B00-B06  Address */
        uint32_t reserved1 : 1; /* B07      Reserved */
    } bit_COMMON;
} I2C_UN_ADR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t D : 8; /* B00-B07  Serial Data */
    } bit_COMMON;
} I2C_UN_DAR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t NF : 5; /* B00-04   Noise Filter Select for Sm/FM/FM+ */
        uint32_t NFH : 3; /* B05-07   Noise Filter Select for Hs */
    } bit_F_I2C_SP1;
} I2C_UN_NFR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TLW : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TLWR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TLW : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TLW2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t THW : 8;
    } bit_F_I2C_SP1;
} I2C_UN_THWR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t THW : 8;
    } bit_F_I2C_SP1;
} I2C_UN_THW2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TBF : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TBFR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TBF : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TBF2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TRS : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TRSR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TRS : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TRS2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TSH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TSHR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TSH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TSH2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TPS : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TPSR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TPS : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TPS2R_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TLWH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TLWRH_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t THWH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_THWRH_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TRSH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TRSRH_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TSHH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TSHRH_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t TPSH : 8;
    } bit_F_I2C_SP1;
} I2C_UN_TPSRH_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t CS : 5; /* B00-B04  Clock Period Select 4-0 */
        uint32_t EN : 1; /* B05      Enable */
        uint32_t FM : 1; /* B06      High Speed Mode */
        uint32_t reserved1 : 1; /* B07      Reserved */
    } bit_F_I2C;
} I2C_UN_CCR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t CS : 6; /* B00-B05  Clock Period Select 10-5 */
        uint32_t TST : 2; /* B06-B07  TST1, TST0 */
    } bit_F_I2C;
} I2C_UN_CSR_t;

typedef union {
    uint8_t DATA;
    struct {
        uint32_t FS : 4; /* B00-B03  Bus Clock Frequency Select 3-0 */
        uint32_t reserved1 : 4; /* B04-B07  Reserved */
    } bit_F_I2C;
} I2C_UN_FSR_t;

/*!
 * \brief SYNQUACER I2C register address definitions
 */
#define I2C_REG_ADDR_BSR (0x00U)
#define I2C_REG_ADDR_BCR (0x04U)
#define I2C_REG_ADDR_CCR (0x08U)
#define I2C_REG_ADDR_ADR (0x0CU)
#define I2C_REG_ADDR_DAR (0x10U)
#define I2C_REG_ADDR_CSR (0x14U)
#define I2C_REG_ADDR_FSR (0x18U)
#define I2C_REG_ADDR_BC2R (0x1CU)

#define I2C_SP1_REG_ADDR_BSR ((0x00U) << 2)
#define I2C_SP1_REG_ADDR_BS2R ((0x01U) << 2)
#define I2C_SP1_REG_ADDR_BCR ((0x02U) << 2)
#define I2C_SP1_REG_ADDR_BC2R ((0x03U) << 2)
#define I2C_SP1_REG_ADDR_ADR ((0x04U) << 2)
#define I2C_SP1_REG_ADDR_DAR ((0x05U) << 2)
#define I2C_SP1_REG_ADDR_NFR ((0x06U) << 2)
#define I2C_SP1_REG_ADDR_TLWR ((0x07U) << 2)
#define I2C_SP1_REG_ADDR_TLW2R ((0x08U) << 2)
#define I2C_SP1_REG_ADDR_THWR ((0x09U) << 2)
#define I2C_SP1_REG_ADDR_THW2R ((0x0AU) << 2)
#define I2C_SP1_REG_ADDR_TBFR ((0x0BU) << 2)
#define I2C_SP1_REG_ADDR_TBF2R ((0x0CU) << 2)
#define I2C_SP1_REG_ADDR_TRSR ((0x0DU) << 2)
#define I2C_SP1_REG_ADDR_TRS2R ((0x0EU) << 2)
#define I2C_SP1_REG_ADDR_TSHR ((0x0FU) << 2)
#define I2C_SP1_REG_ADDR_TSH2R ((0x10U) << 2)
#define I2C_SP1_REG_ADDR_TPSR ((0x11U) << 2)
#define I2C_SP1_REG_ADDR_TPS2R ((0x12U) << 2)
#define I2C_SP1_REG_ADDR_TLWRH ((0x13U) << 2)
#define I2C_SP1_REG_ADDR_THWRH ((0x14U) << 2)
#define I2C_SP1_REG_ADDR_TRSRH ((0x15U) << 2)
#define I2C_SP1_REG_ADDR_TSHRH ((0x16U) << 2)
#define I2C_SP1_REG_ADDR_TPSRH ((0x17U) << 2)

#endif /* INTERNAL_I2C_REG_H */
