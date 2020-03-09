/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      N1SDP I2C register definitions
 */

#ifndef INTERNAL_N1SDP_I2C_H
#define INTERNAL_N1SDP_I2C_H

#include <fwk_macros.h>

#include <stdint.h>

struct i2c_reg {
    /* Control Register */
    FWK_RW  uint16_t CR;
            uint8_t  RESERVED0[2];
    /* Status Register */
    FWK_RW  uint16_t SR;
            uint8_t  RESERVED1[2];
    /* Address Register */
    FWK_RW  uint16_t AR;
            uint8_t  RESERVED2[2];
    /* Data Register */
    FWK_RW  uint16_t DR;
            uint8_t  RESERVED3[2];
    /* Interrupt Status Register */
    FWK_RW  uint16_t ISR;
            uint8_t  RESERVED4[2];
    /* Transfer Size Register */
    FWK_RW  uint8_t  TSR;
            uint8_t  RESERVED5[3];
    /* Slave Monitor Pause Register */
    FWK_RW  uint8_t  SMPR;
            uint8_t  RESERVED6[3];
    /* Timeout Register */
    FWK_RW  uint8_t  TOR;
            uint8_t  RESERVED7[3];
    /* Interrupt Mask Register */
    FWK_RW  uint16_t IMR;
            uint8_t  RESERVED8[2];
    /* Interrupt Enable Register */
    FWK_RW  uint16_t IER;
            uint8_t  RESERVED9[2];
    /* Interrupt Disable Register */
    FWK_RW  uint16_t IDR;
            uint8_t  RESERVED10[2];
    /* Glitch Filter Control Register */
    FWK_RW  uint16_t GFCR;
};

/* Register Field Definitions */

#define I2C_CR_DIV_MASK         0xFF00
#define I2C_CR_DIV_SHIFT        8
#define I2C_CR_DIV_A_MASK       0x000C
#define I2C_CR_DIV_A_SHIFT      14
#define I2C_CR_DIV_B_MASK       0x003F
#define I2C_CR_DIV_B_SHIFT      8
#define I2C_CR_CLRFIFO_MASK     0x0040
#define I2C_CR_CLRFIFO_SHIFT    6
#define I2C_CR_SLVMON_MASK      0x0020
#define I2C_CR_SLVMON_SHIFT     5
#define I2C_CR_HOLD_MASK        0x0010
#define I2C_CR_HOLD_SHIFT       4
#define I2C_CR_ACKEN_MASK       0x0008
#define I2C_CR_ACKEN_SHIFT      3
#define I2C_CR_NEA_MASK         0x0004
#define I2C_CR_NEA_SHIFT        2
#define I2C_CR_MS_MASK          0x0002
#define I2C_CR_MS_SHIFT         1
#define I2C_CR_RW_MASK          0x0001
#define I2C_CR_RW_SHIFT         0

#define I2C_SR_BA_MASK          0x0100
#define I2C_SR_BA_SHIFT         8
#define I2C_SR_RXOVF_MASK       0x0080
#define I2C_SR_RXOVF_SHIFT      7
#define I2C_SR_TXDV_MASK        0x0040
#define I2C_SR_TXDV_SHIFT       6
#define I2C_SR_RXDV_MASK        0x0020
#define I2C_SR_RXDV_SHIFT       5
#define I2C_SR_RXRW_MASK        0x0008
#define I2C_SR_RXRW_SHIFT       3

#define I2C_AR_ADD7_MASK        0x007F
#define I2C_AR_ADD7_SHIFT       0
#define I2C_AR_ADD10_MASK       0x03FF
#define I2C_AR_ADD10_SHIFT      0

#define I2C_DR_DATA_MASK        0x00FF
#define I2C_DR_DATA_SHIFT       0

#define I2C_ISR_MASK            0x02FF
#define I2C_ISR_SHIFT           0
#define I2C_ISR_ARBLOST_MASK    0x0200
#define I2C_ISR_ARBLOST_SHIFT   9
#define I2C_ISR_RXUNF_MASK      0x0080
#define I2C_ISR_RXUNF_SHIFT     7
#define I2C_ISR_TXOVF_MASK      0x0040
#define I2C_ISR_TXOVF_SHIFT     6
#define I2C_ISR_RXOVF_MASK      0x0020
#define I2C_ISR_RXOVF_SHIFT     5
#define I2C_ISR_SLVRDY_MASK     0x0010
#define I2C_ISR_SLVRDY_SHIFT    4
#define I2C_ISR_TO_MASK         0x0008
#define I2C_ISR_TO_SHIFT        3
#define I2C_ISR_NACK_MASK       0x0004
#define I2C_ISR_NACK_SHIFT      2
#define I2C_ISR_DATA_MASK       0x0002
#define I2C_ISR_DATA_SHIFT      1
#define I2C_ISR_COMP_MASK       0x0001
#define I2C_ISR_COMP_SHIFT      0


#define I2C_TSR_SIZE_MASK       0xFF
#define I2C_TSR_SIZE_SHIFT      0

#define I2C_SMPR_PAUSE_MASK     0x0F
#define I2C_SMPR_PAUSE_SHIFT    0

#define I2C_TOR_TIMEOUT_MASK    0xFF
#define I2C_TOR_TIMEOUT_SHIFT   0

#define I2C_IMR_ARBLOST_MASK    0x0200
#define I2C_IMR_ARBLOST_SHIFT   9
#define I2C_IMR_RXUNF_MASK      0x0080
#define I2C_IMR_RXUNF_SHIFT     7
#define I2C_IMR_TXOVF_MASK      0x0040
#define I2C_IMR_TXOVF_SHIFT     6
#define I2C_IMR_RXOVF_MASK      0x0020
#define I2C_IMR_RXOVF_SHIFT     5
#define I2C_IMR_SLVRDY_MASK     0x0010
#define I2C_IMR_SLVRDY_SHIFT    4
#define I2C_IMR_TO_MASK         0x0008
#define I2C_IMR_TO_SHIFT        3
#define I2C_IMR_NACK_MASK       0x0004
#define I2C_IMR_NACK_SHIFT      2
#define I2C_IMR_DATA_MASK       0x0002
#define I2C_IMR_DATA_SHIFT      1
#define I2C_IMR_COMP_MASK       0x0001
#define I2C_IMR_COMP_SHIFT      0

#define I2C_IER_ARBLOST_MASK    0x0200
#define I2C_IER_ARBLOST_SHIFT   9
#define I2C_IER_RXUNF_MASK      0x0080
#define I2C_IER_RXUNF_SHIFT     7
#define I2C_IER_TXOVF_MASK      0x0040
#define I2C_IER_TXOVF_SHIFT     6
#define I2C_IER_RXOVF_MASK      0x0020
#define I2C_IER_RXOVF_SHIFT     5
#define I2C_IER_SLVRDY_MASK     0x0010
#define I2C_IER_SLVRDY_SHIFT    4
#define I2C_IER_TO_MASK         0x0008
#define I2C_IER_TO_SHIFT        3
#define I2C_IER_NACK_MASK       0x0004
#define I2C_IER_NACK_SHIFT      2
#define I2C_IER_DATA_MASK       0x0002
#define I2C_IER_DATA_SHIFT      1
#define I2C_IER_COMP_MASK       0x0001
#define I2C_IER_COMP_SHIFT      0

#define I2C_IDR_ARBLOST_MASK    0x0200
#define I2C_IDR_ARBLOST_SHIFT   9
#define I2C_IDR_RXUNF_MASK      0x0080
#define I2C_IDR_RXUNF_SHIFT     7
#define I2C_IDR_TXOVF_MASK      0x0040
#define I2C_IDR_TXOVF_SHIFT     6
#define I2C_IDR_RXOVF_MASK      0x0020
#define I2C_IDR_RXOVF_SHIFT     5
#define I2C_IDR_SLVRDY_MASK     0x0010
#define I2C_IDR_SLVRDY_SHIFT    4
#define I2C_IDR_TO_MASK         0x0008
#define I2C_IDR_TO_SHIFT        3
#define I2C_IDR_NACK_MASK       0x0004
#define I2C_IDR_NACK_SHIFT      2
#define I2C_IDR_DATA_MASK       0x0002
#define I2C_IDR_DATA_SHIFT      1
#define I2C_IDR_COMP_MASK       0x0001
#define I2C_IDR_COMP_SHIFT      0

#define I2C_GFCR_DEPTH_MASK     0xFFFF
#define I2C_GFCR_DEPTH_SHIFT    0

/* I2C Driver Macros */

#define I2C_RW_WRITE     0
#define I2C_RW_READ      1

#define I2C_MS_SLAVE     0
#define I2C_MS_MASTER    1

#define I2C_NEA_10BIT    0
#define I2C_NEA_7BIT     1

#define I2C_ACKEN_OFF    0
#define I2C_ACKEN_ON     1

#define I2C_HOLD_OFF     0
#define I2C_HOLD_ON      1

#define I2C_CLRFIFO_OFF  0
#define I2C_CLRFIFO_ON   1

#define I2C_TSR_TANSFER_SIZE 0xF

#endif /* INTERNAL_N1SDP_I2C_H */
