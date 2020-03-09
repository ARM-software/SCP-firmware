
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN600 CCIX Configuration Interface
 */

#ifndef INTERNAL_CMN600_CCIX_H
#define INTERNAL_CMN600_CCIX_H

#include <internal/cmn600_ctx.h>

#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * CMN600 CCIX Setup Function
 */
int ccix_setup(struct cmn600_ctx *ctx, void *remote_config);

/*
 * CMN600 CCIX Exchange Protocol Credit Function
 */
int ccix_exchange_protocol_credit(struct cmn600_ctx *ctx, uint8_t link_id);

/*
 *  CMN600 CCIX Enter system Coherency Function
 */
int ccix_enter_system_coherency(struct cmn600_ctx *ctx, uint8_t link_id);

/*
 *  CMN600 CCIX Enter DVM domain Function
 */
int ccix_enter_dvm_domain(struct cmn600_ctx *ctx, uint8_t link_id);

/*
 * CMN600 CCIX get Capabilities Function
 */
void ccix_capabilities_get(struct cmn600_ctx *ctx);

/*
 * CCIX Gateway (CXG) protocol link control & status registers
 */
struct cxg_link_regs {
    FWK_RW uint64_t     CXG_PRTCL_LINK_CTRL;
    FWK_R  uint64_t     CXG_PRTCL_LINK_STATUS;
};

/*
 * CCIX Gateway (CXG) Home Agent (HA) registers
 */
struct cmn600_cxg_ha_reg {
    FWK_R  uint64_t     CXG_HA_NODE_INFO;
    FWK_RW uint64_t     CXG_HA_ID;
           uint8_t      RESERVED0[0x80-0x10];
    FWK_R  uint64_t     CXG_HA_CHILD_INFO;
           uint8_t      RESERVED1[0x900-0x88];
    FWK_R  uint64_t     CXG_HA_UNIT_INFO;
           uint8_t      RESERVED2[0x980-0x908];
    FWK_RW uint64_t     CXG_HA_SEC_REG_GRP_OVERRIDE;
           uint8_t      RESERVED3[0xA08-0x988];
    FWK_RW uint64_t     CXG_HA_AUX_CTRL;
           uint8_t      RESERVED4[0xC00-0xA10];
    FWK_RW uint64_t     CXG_HA_RNF_RAID_TO_LDID_REG[8];
    FWK_RW uint64_t     CXG_HA_AGENTID_TO_LINKID_REG[8];
           uint8_t      RESERVED5[0xD00-0xC80];
    FWK_RW uint64_t     CXG_HA_AGENTID_TO_LINKID_VAL;
    FWK_RW uint64_t     CXG_HA_RNF_RAID_TO_LDID_VAL;
           uint8_t      RESERVED6[0x1000-0xD10];
           struct cxg_link_regs LINK_REGS[3];
           uint8_t      RESERVED7[0x2000-0x1030];
    FWK_RW uint64_t     CXG_HA_PMU_EVENT_SEL;
};

/*
 * CCIX Gateway (CXG) Requesting Agent (RA) registers
 */
struct cmn600_cxg_ra_reg {
    FWK_R  uint64_t     CXG_RA_NODE_INFO;
           uint8_t      RESERVED0[0x80-0x8];
    FWK_R  uint64_t     CXG_RA_CHILD_INFO;
           uint8_t      RESERVED1[0x900-0x88];
    FWK_R  uint64_t     CXG_RA_UNIT_INFO;
           uint8_t      RESERVED2[0x980-0x908];
    FWK_RW uint64_t     CXG_RA_SEC_REG_GRP_OVERRIDE;
           uint8_t      RESERVED3[0xA00-0x988];
    FWK_RW uint64_t     CXG_RA_CFG_CTRL;
    FWK_RW uint64_t     CXG_RA_AUX_CTRL;
           uint8_t      RESERVED4[0xDA8-0xA10];
    FWK_RW uint64_t     CXG_RA_SAM_ADDR_REGION_REG[8];
           uint8_t      RESERVED5[0xE00-0xDE8];
    FWK_RW uint64_t     CXG_RA_SAM_MEM_REGION_LIMIT_REG[8];
           uint8_t      RESERVED6[0xE60-0xE40];
    FWK_RW uint64_t     CXG_RA_AGENTID_TO_LINKID_REG[8];
    FWK_RW uint64_t     CXG_RA_RNF_LDID_TO_RAID_REG[8];
    FWK_RW uint64_t     CXG_RA_RNI_LDID_TO_RAID_REG[4];
    FWK_RW uint64_t     CXG_RA_RND_LDID_TO_RAID_REG[4];
    FWK_RW uint64_t     CXG_RA_AGENTID_TO_LINKID_VAL;
    FWK_RW uint64_t     CXG_RA_RNF_LDID_TO_RAID_VAL;
    FWK_RW uint64_t     CXG_RA_RNI_LDID_TO_RAID_VAL;
    FWK_RW uint64_t     CXG_RA_RND_LDID_TO_RAID_VAL;
           uint8_t      RESERVED7[0x1000-0xF40];
           struct cxg_link_regs LINK_REGS[3];
           uint8_t      RESERVED8[0x2000-0x1030];
    FWK_RW uint64_t     CXG_RA_PMU_EVENT_SEL;
};

/*
 * CCIX Gateway (CXG) Link Agent (LA) registers
 */
struct cmn600_cxla_reg {
    FWK_R  uint64_t     CXLA_NODE_INFO;
           uint8_t      RESERVED0[0x80-0x8];
    FWK_R  uint64_t     CXLA_CHILD_INFO;
           uint8_t      RESERVED1[0x900-0x88];
    FWK_R  uint64_t     CXLA_UNIT_INFO;
           uint8_t      RESERVED2[0x980-0x908];
    FWK_RW uint64_t     CXLA_SEC_REG_GRP_OVERRIDE;
           uint8_t      RESERVED3[0xA08-0x988];
    FWK_RW uint64_t     CXLA_AUX_CTRL;
           uint8_t      RESERVED4[0xC00-0xA10];
    FWK_R  uint64_t     CXLA_CCIX_PROP_CAPABILITIES;
    FWK_RW uint64_t     CXLA_CCIX_PROP_CONFIGURED;
    FWK_R  uint64_t     CXLA_TX_CXS_ATTR_CAPABILITIES;
    FWK_R  uint64_t     CXLA_RX_CXS_ATTR_CAPABILITIES;
           uint8_t      RESERVED5[0xC30-0xC20];
    FWK_RW uint64_t     CXLA_AGENTID_TO_LINKID_REG[8];
    FWK_RW uint64_t     CXLA_AGENTID_TO_LINKID_VAL;
    FWK_RW uint64_t     CXLA_LINKID_TO_PCIE_BUS_NUM;
    FWK_RW uint64_t     CXLA_PCIE_HDR_FIELDS;
};



/*
 * CCIX Definitions
 */

/*
 * CCIX Link UP stages
 */
enum cxg_link_up_wait_cond {
    CXG_LINK_CTRL_EN_BIT_SET,
    CXG_LINK_CTRL_UP_BIT_CLR,
    CXG_LINK_STATUS_DWN_BIT_SET,
    CXG_LINK_STATUS_DWN_BIT_CLR,
    CXG_LINK_STATUS_ACK_BIT_SET,
    CXG_LINK_STATUS_ACK_BIT_CLR,
    CXG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET,
    CXG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET,
    CXG_LINK_UP_SEQ_COUNT,
};

/*
 * Structure defining data to be passed to timer API
 */
struct cxg_wait_condition_data {
    struct cmn600_ctx *ctx;
    uint8_t link_id;
    enum cxg_link_up_wait_cond cond;
};

/* CCIX Gateway (CXG) Request Agent (RA) defines */

#define CXG_RA_SAM_HA_TGT_ID_SHIFT_VAL            52
#define CXG_RA_RNF_LDID_TO_RAID_REG_OFFSET        0xEA0
#define CXG_RA_RNI_LDID_TO_RAID_REG_OFFSET        0xEE0
#define CXG_RA_RND_LDID_TO_RAID_REG_OFFSET        0xF00
#define CXG_RA_RNF_RAID_VALID_REG_OFFSET          0xF28
#define CXG_RA_RNI_RAID_VALID_REG_OFFSET          0xF30
#define CXG_RA_RND_RAID_VALID_REG_OFFSET          0xF38
#define CXG_RA_AGENTID_TO_LINKID_OFFSET           0xE60
#define CXG_RA_AGENTID_TO_LINKID_VAL_OFFSET       0xF20
#define CXG_RA_REQUEST_TRACKER_DEPTH_MASK         UINT64_C(0x0000000001FF0000)
#define CXG_RA_REQUEST_TRACKER_DEPTH_VAL          16
#define CXG_RA_UNIT_INFO_SMP_MODE_RO_MASK         (UINT64_C(1) << 61)
#define CXG_RA_AUX_CTRL_SMP_MODE_RW_SHIFT_VAL     (16)

/* CCIX Gateway (CXG) Home Agent (HA) defines */

#define CXG_HA_AGENTID_TO_LINKID_OFFSET           0xC40
#define CXG_HA_AGENTID_TO_LINKID_VAL_OFFSET       0xD00
#define CXG_HA_RAID_TO_LDID_OFFSET                (0xC00)
#define CXG_HA_RAID_TO_LDID_VALID_OFFSET          (0xD08)
#define CXG_HA_RAID_TO_LDID_RNF_MASK              (0x80)
#define CXG_HA_LDID_TO_RAID_OFFSET                0xC00
#define CXG_HA_SNOOP_TRACKER_DEPTH_MASK           UINT64_C(0x00001FF000000000)
#define CXG_HA_SNOOP_TRACKER_DEPTH_VAL            36
#define CXG_HA_WDB_DEPTH_MASK                     UINT64_C(0x0000000007FC0000)
#define CXG_HA_WDB_DEPTH_VAL                      18
#define CXG_HA_UNIT_INFO_SMP_MODE_RO_MASK         (UINT64_C(1) << 63)
#define CXG_HA_AUX_CTRL_SMP_MODE_RW_SHIFT_VAL     (16)

/* CCIX Gateway (CXG) Link Agent (LA) defines */

#define CXLA_AGENTID_TO_LINKID_OFFSET             0xC30
#define CXLA_AGENTID_TO_LINKID_VAL_OFFSET         0xC70

#define CXLA_CCIX_PROP_MSG_PACK_SHIFT_MASK        UINT64_C(0x0000000000000400)
#define CXLA_CCIX_PROP_MSG_PACK_SHIFT_VAL         10

#define CXLA_CCIX_PROP_MAX_PACK_SIZE_MASK         UINT64_C(0x0000000000000380)
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_SHIFT_VAL    7
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_128          0
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_256          1
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_512          2

#define CXLA_PCIE_HDR_TRAFFIC_CLASS_MASK          UINT64_C(0x0000000000007000)
#define CXLA_PCIE_HDR_VENDOR_ID_MASK              UINT64_C(0x0000FFFF00000000)

#define CXLA_PCIE_HDR_TRAFFIC_CLASS_SHIFT_VAL     12
#define CXLA_PCIE_HDR_VENDOR_ID_SHIFT_VAL         32

#define CXLA_AUX_CTRL_SMP_MODE_SHIFT_VAL          (47)

/* CCIX Gateway (CXG) link control & status defines */

#define CXG_LINK_CTRL_EN_MASK                     UINT64_C(0x0000000000000001)
#define CXG_LINK_CTRL_REQ_MASK                    UINT64_C(0x0000000000000002)
#define CXG_LINK_CTRL_UP_MASK                     UINT64_C(0x0000000000000004)
#define CXG_LINK_CTRL_DVMDOMAIN_REQ_MASK          UINT64_C(0x0000000000000008)
#define CXG_LINK_STATUS_ACK_MASK                  UINT64_C(0x0000000000000001)
#define CXG_LINK_STATUS_DOWN_MASK                 UINT64_C(0x0000000000000002)
#define CXG_LINK_STATUS_DVMDOMAIN_ACK_MASK        UINT64_C(0x0000000000000004)
#define CXG_PRTCL_LINK_CTRL_TIMEOUT               UINT32_C(100)
#define CXG_PRTCL_LINK_DVMDOMAIN_TIMEOUT          UINT32_C(100)

#define RAID_RNF_BIT_SHIFT_VAL                    7
#define HNF_RN_PHYS_NODE_ID_SHIFT_VAL             16
#define HNF_RN_PHYS_RN_ID_VALID_SHIFT_VAL         31
#define HNF_RN_PHYS_RN_LOCAL_REMOTE_SHIFT_VAL     16
#define NUM_BITS_RESERVED_FOR_RAID                8
#define NUM_BITS_RESERVED_FOR_LINK_ID             8
#define NUM_BITS_RESERVED_FOR_LDID                8
#define NUM_BITS_RESERVED_FOR_PHYS_ID             32
#define HNF_RN_PHYS_ID_OFFSET                     0xD28
#define LOCAL_CCIX_NODE                           0
#define REMOTE_CCIX_NODE                          1
#define SAM_ADDR_HOME_AGENT_ID_SHIFT              (52)
#define SAM_ADDR_REG_VALID_MASK                   UINT64_C(0x8000000000000000)
#define PCIE_OPT_HDR_MASK                         (0x1ULL << 6)
#define CTL_NUM_SNPCRDS_MASK                      (0xF << 4)
#define CCIX_VENDER_ID                            (0x1E2C)

#endif /* INTERNAL_CMN600_CCIX_H */
