/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN-700 CCG Configuration Interface
 */

#ifndef CMN700_CCG_H
#define CMN700_CCG_H

#include <internal/cmn700_ctx.h>

#include <mod_cmn700.h>

int ccg_setup(
    const unsigned int chip_id,
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config);

int ccg_exchange_protocol_credit(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config);

int ccg_enter_system_coherency(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config);

int ccg_enter_dvm_domain(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config);

/*
 * CCG Link UP stages
 */
enum ccg_link_up_wait_cond {
    CCG_LINK_CTRL_EN_BIT_SET,
    CCG_LINK_CTRL_UP_BIT_CLR,
    CCG_LINK_STATUS_DWN_BIT_SET,
    CCG_LINK_STATUS_DWN_BIT_CLR,
    CCG_LINK_STATUS_ACK_BIT_SET,
    CCG_LINK_STATUS_ACK_BIT_CLR,
    CCG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET,
    CCG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET,
    CCG_LINK_UP_SEQ_COUNT,
};

/*
 * Structure defining data to be passed to timer API
 */
struct ccg_wait_condition_data {
    struct cmn700_device_ctx *ctx;
    uint8_t linkid;
    enum ccg_link_up_wait_cond cond;
};

/* CCG Home Agent (HA) defines */
#define CCG_HA_RAID_TO_LDID_RNF_MASK (0x4000)

#define CCLA_CCG_PROP_MAX_PACK_SIZE_MASK      UINT64_C(0x0000000000000380)
#define CCLA_CCG_PROP_MAX_PACK_SIZE_SHIFT_VAL 7
#define CCLA_CCG_PROP_MAX_PACK_SIZE_128       0
#define CCLA_CCG_PROP_MAX_PACK_SIZE_256       1
#define CCLA_CCG_PROP_MAX_PACK_SIZE_512       2

/* CCG link control & status defines */

#define CCG_LINK_CTRL_EN_MASK              UINT64_C(0x0000000000000001)
#define CCG_LINK_CTRL_REQ_MASK             UINT64_C(0x0000000000000002)
#define CCG_LINK_CTRL_UP_MASK              UINT64_C(0x0000000000000004)
#define CCG_LINK_CTRL_DVMDOMAIN_REQ_MASK   UINT64_C(0x0000000000000008)
#define CCG_LINK_STATUS_ACK_MASK           UINT64_C(0x0000000000000001)
#define CCG_LINK_STATUS_DOWN_MASK          UINT64_C(0x0000000000000002)
#define CCG_LINK_STATUS_DVMDOMAIN_ACK_MASK UINT64_C(0x0000000000000004)
#define CCG_CCPRTCL_LINK_CTRL_TIMEOUT      UINT32_C(100)
#define CCG_CCPRTCL_LINK_DVMDOMAIN_TIMEOUT UINT32_C(100)

/* SMP Mode related defines */
#define CCG_RA_CCPRTCL_LINK_CTRL_SMP_MODE_EN_SHIFT_VAL 16
#define CCG_HA_CCPRTCL_LINK_CTRL_SMP_MODE_EN_SHIFT_VAL 16

/* ULL to ULL Mode related defines */
#define CCLA_ULL_CTL_ULL_TO_ULL_MODE_EN_SHIFT_VAL 1
#define CCLA_ULL_CTL_SEND_VD_INIT_SHIFT_VAL       0
#define CCLA_ULL_STATUS_SEND_RX_ULL_STATE_MASK    UINT64_C(0x2)
#define CCLA_ULL_STATUS_SEND_TX_ULL_STATE_MASK    UINT64_C(0x1)
#define CCLA_ULL_STATUS_TIMEOUT                   UINT32_C(100)

#define HNF_RN_PHYS_RN_ID_VALID_SHIFT_VAL     31
#define HNF_RN_PHYS_RN_LOCAL_REMOTE_SHIFT_VAL 16

/* 10 bits for RAID, 5 reserved bits, 1 bit for valid */
#define NUM_BITS_RESERVED_FOR_RAID    16
#define LDID_TO_EXP_RAID_VALID_MASK   (UINT64_C(1) << 15)
#define NUM_BITS_RESERVED_FOR_LINKID  8
#define NUM_BITS_RESERVED_FOR_LDID    16
#define EXP_RAID_TO_LDID_VALID_MASK   (UINT64_C(1) << 15)
#define NUM_BITS_RESERVED_FOR_PHYS_ID 32
#define LOCAL_CCG_NODE                0
#define REMOTE_CCG_NODE               1
#define SAM_ADDR_TARGET_HAID_SHIFT    (52)
#define SAM_ADDR_REG_VALID_MASK       UINT64_C(0x8000000000000000)

/*
 * Used by RNSAM and HNSAM CPA registers
 *
 * CPA - CCG Port Aggregation
 * PAG - Port Aggregation Group
 * GRPID - Group ID
 */

#define HNF_RN_PHYS_CPA_GRP_RA_SHIFT_VAL 17
#define HNF_RN_PHYS_CPA_EN_RA_SHIFT_VAL  30

#define CML_PORT_AGGR_MODE_CTRL_REGIONS_PER_GROUP    9
#define CML_PORT_AGGR_MODE_CTRL_PAG_WIDTH_PER_REGION 6
#define CML_PORT_AGGR_MODE_CTRL_PAG_GRPID_OFFSET     1
#define CML_PORT_AGGR_CTRL_NUM_CCG_PAG_WIDTH         12
#define CMN_PORT_AGGR_GRP_PAG_TGTID_PER_GROUP        5
#define CMN_PORT_AGGR_GRP_PAG_TGTID_WIDTH            12
#define CMN_PORT_AGGR_GRP_PAG_TGTID_WIDTH_PER_GROUP  60
#define NUM_PORTS_PER_CPA_GROUP                      2

#endif /* CMN700_CCG_H */
