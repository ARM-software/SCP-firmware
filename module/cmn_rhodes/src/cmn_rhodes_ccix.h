/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN-Rhodes CCIX Configuration Interface
 */

#ifndef CMN_RHODES_CCIX_H
#define CMN_RHODES_CCIX_H

#include <mod_cmn_rhodes.h>
#include <internal/cmn_rhodes_ctx.h>

int ccix_setup(const unsigned int chip_id, struct cmn_rhodes_device_ctx *ctx,
        const struct mod_cmn_rhodes_ccix_config *ccix_config);

int ccix_exchange_protocol_credit(struct cmn_rhodes_device_ctx *ctx,
        const struct mod_cmn_rhodes_ccix_config *ccix_config);

int ccix_enter_system_coherency(struct cmn_rhodes_device_ctx *ctx,
        const struct mod_cmn_rhodes_ccix_config *ccix_config);

int ccix_enter_dvm_domain(struct cmn_rhodes_device_ctx *ctx,
        const struct mod_cmn_rhodes_ccix_config *ccix_config);

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
    struct cmn_rhodes_device_ctx *ctx;
    uint8_t linkid;
    enum cxg_link_up_wait_cond cond;
};

/* CCIX Gateway (CXG) Home Agent (HA) defines */
#define CXG_HA_RAID_TO_LDID_RNF_MASK              (0x80)

#define CXLA_CCIX_PROP_MAX_PACK_SIZE_MASK         UINT64_C(0x0000000000000380)
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_SHIFT_VAL    7
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_128          0
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_256          1
#define CXLA_CCIX_PROP_MAX_PACK_SIZE_512          2

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

/* SMP Mode related defines */
#define CXG_RA_AUX_CTRL_SMP_MODE_EN_SHIFT_VAL     16
#define CXG_HA_AUX_CTRL_SMP_MODE_EN_SHIFT_VAL     16
#define CXLA_AUX_CTRL_SMP_MODE_EN_SHIFT_VAL       47

#define HNF_RN_PHYS_RN_ID_VALID_SHIFT_VAL         31
#define HNF_RN_PHYS_RN_LOCAL_REMOTE_SHIFT_VAL     16
#define NUM_BITS_RESERVED_FOR_RAID                8
#define NUM_BITS_RESERVED_FOR_LINKID              8
#define NUM_BITS_RESERVED_FOR_LDID                8
#define NUM_BITS_RESERVED_FOR_PHYS_ID             32
#define LOCAL_CCIX_NODE                           0
#define REMOTE_CCIX_NODE                          1
#define SAM_ADDR_TARGET_HAID_SHIFT                (52)
#define SAM_ADDR_REG_VALID_MASK                   UINT64_C(0x8000000000000000)

#endif /* CMN_RHODES_CCIX_H */
