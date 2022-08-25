/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_MHU3_H
#define INTERNAL_MHU3_H

#include <mod_mhu3.h>

#include <fwk_macros.h>

#include <stdint.h>

#define MHU3_MAX_DOORBELL_CHANNELS UINT32_C(128)

/*
 * Useful macros
 */
#define MHU3_MASK_GENERATE(BITSTART, LENGTH) \
    (((1 << (LENGTH)) - 1) << ((BITSTART) + 1 - (LENGTH)))
#define MHU3_MASKED_RECOVER(MASKED, BITSTART, LENGTH) \
    ((MASKED) >> ((BITSTART) + 1 - (LENGTH)))

/*
 * Top level base addresses
 */
#define MHU3_PBX_PDBCW_PAGE_OFFSET UINT32_C(0x00001000)
#define MHU3_PBX_PFCW_PAGE_OFFSET  UINT32_C(0x00003000)

#define MHU3_MBX_MDBCW_PAGE_OFFSET UINT32_C(0x00001000)
#define MHU3_MBX_MDFCW_PAGE_OFFSET UINT32_C(0x00003000)

#define FCH_WS_64BIT 64U
#define FCH_WS_32BIT 32U

enum mhu3_reg_settings {
    MHU3_OP_REQ = 1U << 0U,
    MHU3_AUTO_OP_SPT = 1U << 0U,
};

/*!
 * \brief MHU3 Postbox(PBX) Register Definitions
 */

struct mhu3_pbx_reg {
    /*! Postbox control page */
    FWK_R uint32_t MHU_BLK_ID;
    uint8_t RESERVED1[0x10 - 0x04];
    FWK_R uint32_t PBX_FEAT_SPT0;
    FWK_R uint32_t PBX_FEAT_SPT1;
    uint8_t RESERVED2[0x20 - 0x18];
    FWK_R uint32_t PBX_DBCH_CFG0;
    uint8_t RESERVED3[0x30 - 0x24];
    FWK_R uint32_t PBX_FFCH_CFG0;
    uint8_t RESERVED4[0x40 - 0x34];
    FWK_R uint32_t PBX_FCH_CFG0;
    uint8_t RESERVED5[0x50 - 0x44];
    FWK_R uint32_t PBX_DCH_CFG0;
    uint8_t RESERVED6[0x100 - 0x54];
    FWK_RW uint32_t PBX_CTRL;
    uint8_t RESERVED7[0x150 - 0x104];
    FWK_RW uint32_t PBX_DMA_CTRL;
    FWK_R uint32_t PBX_DMA_ST;
    FWK_RW uint64_t PBX_DMA_CDL_BASE;
    FWK_RW uint32_t PBX_DMA_CDL_PROP;
    uint8_t RESERVED8[0x400 - 0x164];
    FWK_R uint32_t PBX_DBCH_INT_ST[(0x410 - 0x400) >> 2];
    FWK_R uint32_t PBX_FFCH_INT_ST[(0x430 - 0x410) >> 2];
    FWK_R uint32_t PBX_DCH_INT_ST;
    uint8_t RESERVED9[0xFC8 - 0x434];
    FWK_R uint32_t IIDR;
    FWK_R uint32_t AIDR;
    FWK_R uint32_t IMPL_DEF_ID[4 * 11];
};

struct mhu3_pbx_pdbcw_reg {
    FWK_R uint32_t PDBCW_ST;
    uint8_t RESERVED1[0xC - 0x4];
    FWK_W uint32_t PDBCW_SET;
    FWK_R uint32_t PDBCW_INT_ST;
    FWK_W uint32_t PDBCW_INT_CLR;
    FWK_RW uint32_t PDBCW_INT_EN;
    FWK_RW uint32_t PDBCW_CTRL;
};

struct mhu3_mbx_reg {
    /*! Mailbox control page */
    FWK_R uint32_t MHU_BLK_ID;
    uint8_t RESERVED1[0x10 - 0x04];
    FWK_R uint32_t MBX_FEAT_SPT0;
    FWK_R uint32_t MBX_FEAT_SPT1;
    uint8_t RESERVED2[0x20 - 0x18];
    FWK_R uint32_t MBX_DBCH_CFG0;
    uint8_t RESERVED3[0x30 - 0x24];
    FWK_R uint32_t MBX_FFCH_CFG0;
    uint8_t RESERVED4[0x40 - 0x34];
    FWK_R uint32_t MBX_FCH_CFG0;
    uint8_t RESERVED5[0x50 - 0x44];
    FWK_R uint32_t MBX_DCH_CFG0;
    uint8_t RESERVED6[0x100 - 0x54];
    FWK_RW uint32_t MBX_CTRL;
    uint8_t RESERVED7[0x140 - 0x104];
    FWK_RW uint32_t MBX_FCH_CTRL;
    FWK_RW uint32_t MBX_FCG_INT_EN;
    uint8_t RESERVED8[0x150 - 0x148];
    FWK_RW uint32_t MBX_DMA_CTRL;
    FWK_R uint32_t MBX_DMA_ST;
    FWK_RW uint64_t MBX_DMA_CDL_BASE;
    FWK_RW uint32_t MBX_DMA_CDL_PROP;
    uint8_t RESERVED9[0x400 - 0x164];
    FWK_R uint32_t MBX_DBCH_INT_ST[(0x410 - 0x400) >> 2];
    FWK_R uint32_t MBX_FFCH_INT_ST[(0x420 - 0x410) >> 2];
    FWK_R uint32_t MBX_FCG_INT_ST;
    uint8_t RESERVED10[0x430 - 0x424];
    FWK_R uint32_t MBX_FCH_GRP_INT_ST[(0x4B0 - 0x430) >> 2];
    FWK_R uint32_t MBX_DCH_INT_ST;
    uint8_t RESERVED11[0xFC8 - 0x4B4];
    FWK_R uint32_t IIDR;
    FWK_R uint32_t AIDR;
    FWK_R uint32_t IMPL_DEF_ID[4 * 11];
};

struct mhu3_mbx_mdbcw_reg {
    FWK_R uint32_t MDBCW_ST;
    FWK_R uint32_t MDBCW_ST_MSK;
    FWK_RW uint32_t MDBCW_CLR;
    uint8_t RESERVED1[0x10 - 0x0C];
    FWK_R uint32_t MDBCW_MSK_ST;
    FWK_RW uint32_t MDBCW_MSK_SET;
    FWK_RW uint32_t MDBCW_MSK_CLR;
    FWK_RW uint32_t MDBCW_CTRL;
};

/* Fast Channel Configuration 0 Number of Fast Channels Start Bit */
#define MHU3_FCH_CFG0_NUM_FCH_BITSTART 9
/* Fast Channel Configuration 0 Number of Fast Channels Length */
#define MHU3_FCH_CFG0_NUM_FCH_LEN 10
/* Fast Channel Configuration 0 Number of Fast Channels Mask */
#define MHU3_FCH_CFG0_NUM_FCH_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FCH_CFG0_NUM_FCH_BITSTART, MHU3_FCH_CFG0_NUM_FCH_LEN))

/* Fast Channel Configuration 0 Number of Fast Channel Groups Start Bit */
#define MHU3_FCH_CFG0_NUM_FCG_BITSTART 15
/* Fast Channel Configuration 0 Number of Fast Channel Groups Length */
#define MHU3_FCH_CFG0_NUM_FCG_LEN 5
/* Fast Channel Configuration 0 Number of Fast Channel Groups Mask */
#define MHU3_FCH_CFG0_NUM_FCG_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FCH_CFG0_NUM_FCG_BITSTART, MHU3_FCH_CFG0_NUM_FCG_LEN))

/*
 * Fast Channel Configuration 0 Number of Fast Channels per Fast Channel Group
 * Start Bit
 */
#define MHU3_FCH_CFG0_NUM_FCH_PER_GRP_BITSTART 20
/*
 * Fast Channel Configuration 0 Number of Fast Channels per Fast Channel Group
 * Length
 */
#define MHU3_FCH_CFG0_NUM_FCH_PER_GRP_LEN 5
/*
 * Fast Channel Configuration 0 Number of Fast Channels per Fast Channel Group
 * Mask
 */
#define MHU3_FCH_CFG0_NUM_FCH_PER_GRP_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FCH_CFG0_NUM_FCH_PER_GRP_BITSTART, \
        MHU3_FCH_CFG0_NUM_FCH_PER_GRP_LEN))

/* Fast Channel Configuration 0 Fast Channel Word Size Start Bit */
#define MHU3_FCH_CFG0_FCH_WS_BITSTART 28
/* Fast Channel Configuration 0 Fast Channel Word Size Length */
#define MHU3_FCH_CFG0_FCH_WS_LEN 8
/* Fast Channel Configuration 0 Fast Channel Word Size Mask */
#define MHU3_FCH_CFG0_FCH_WS_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FCH_CFG0_FCH_WS_BITSTART, MHU3_FCH_CFG0_FCH_WS_LEN))

/* Feature Support 0 Doorbell Extension Support Start Bit */
#define MHU3_FEAT_SPT0_DBE_SPT_BITSTART 3
/* Feature Support 0 Doorbell Extension Support Length */
#define MHU3_FEAT_SPT0_DBE_SPT_LEN 4
/* Feature Support 0 Doorbell Extension Support Mask */
#define MHU3_FEAT_SPT0_DBE_SPT_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FEAT_SPT0_DBE_SPT_BITSTART, MHU3_FEAT_SPT0_DBE_SPT_LEN))

/* Feature Support 0 FIFO Extension Support Start Bit */
#define MHU3_FEAT_SPT0_FE_SPT_BITSTART 7
/* Feature Support 0 FIFO Extension Support Length */
#define MHU3_FEAT_SPT0_FE_SPT_LEN 4
/* Feature Support 0 FIFO Extension Support Mask */
#define MHU3_FEAT_SPT0_FE_SPT_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FEAT_SPT0_FE_SPT_BITSTART, MHU3_FEAT_SPT0_FE_SPT_LEN))

/* Feature Support 0 Fast Channel Extension Support Start Bit */
#define MHU3_FEAT_SPT0_FCE_SPT_BITSTART 11
/* Feature Support 0 Fast Channel Extension Support Length */
#define MHU3_FEAT_SPT0_FCE_SPT_LEN 4
/* Feature Support 0 Fast Channel Extension Support Mask */
#define MHU3_FEAT_SPT0_FCE_SPT_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FEAT_SPT0_FCE_SPT_BITSTART, MHU3_FEAT_SPT0_FCE_SPT_LEN))

/* Feature Support 0 TrustZone Extension Support Start Bit */
#define MHU3_FEAT_SPT0_TZE_SPT_BITSTART 15
/* Feature Support 0 TrustZone Extension Support Length */
#define MHU3_FEAT_SPT0_TZE_SPT_LEN 4
/* Feature Support 0 TrustZone Extension Support Mask */
#define MHU3_FEAT_SPT0_TZE_SPT_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FEAT_SPT0_TZE_SPT_BITSTART, MHU3_FEAT_SPT0_TZE_SPT_LEN))

/* Feature Support 0 Realm Management Extension Support Start Bit */
#define MHU3_FEAT_SPT0_RME_SPT_BITSTART 19
/* Feature Support 0 Realm Management Extension Support Length */
#define MHU3_FEAT_SPT0_RME_SPT_LEN 4
/* Feature Support 0 Realm Management Extension Support Mask */
#define MHU3_FEAT_SPT0_RME_SPT_MASK \
    (MHU3_MASK_GENERATE( \
        MHU3_FEAT_SPT0_RME_SPT_BITSTART, MHU3_FEAT_SPT0_RME_SPT_LEN))

#endif /* INTERNAL_MHU3_H */
