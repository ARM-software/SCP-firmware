/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing HN-S node.
 */

#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_hns_reg.h>
#include <internal/cmn_cyprus_reg.h>
#include <internal/cmn_cyprus_rnsam_reg.h>

#include <fwk_assert.h>
#include <fwk_math.h>

#include <stdbool.h>
#include <stdint.h>

/* HN-S SAM_CONTROL */
#define HNS_SAM_CONTROL_SN_MODE_POS(sn_mode)   (36 + sn_mode)
#define HNS_SAM_CONTROL_SN_NODE_ID_POS(sn_idx) (sn_idx * 12)
#define HNS_SAM_CONTROL_SN_NODE_ID_MASK        (0x7FF)

/* Only 3 target SN node IDs can be programmed in a HN-S node */
#define HNS_SAM_CONTROL_SN_IDX_COUNT 3

/* HN-S SAM Top Address Bit */
#define HNS_SAM_CONTROL_TOP_ADDR_BIT_POS(x) (40 + (x * 8))

/* HN-S SAM */
#define HNS_UNIT_INFO_HNSAM_RCOMP_EN_MASK  (0x20000000)
#define HNS_UNIT_INFO_HNSAM_RCOMP_EN_POS   29
#define HNS_UNIT_INFO_HNSAM_RCOMP_LSB_MASK (0x7C0000000)
#define HNS_UNIT_INFO_HNSAM_RCOMP_LSB_POS  30
#define HNS_SAM_MEMREGION_SIZE_POS         12
#define HNS_SAM_MEMREGION_BASE_POS         20
#define HNS_SAM_MEMREGION_VALID            UINT64_C(0x8000000000000000)
#define HNS_SAM_MEMREGION_SN_NODE_ID_MASK  (0x7FF)

/* HN-S Power Policy */
#define HNS_PWPR_DYN_EN_POS  8
#define HNS_PWPR_OP_MODE_POS 4
#define HNS_PWPR_POLICY_POS  0

/*
 * Minimum size of hashed and non-hashed regions when RCOMP mode
 * is disabled is 64MB. In this mode, the base and the size of the
 * programmed region is used for comparison.
 */
#define HNSAM_REGION_MIN_SIZE (64 * FWK_MIB)

/* HN-S RN_CLUSTER_PHYSID */
#define HNS_RN_PHYS_RN_ID_VALID_POS     31
#define HNS_RN_PHYS_RN_LOCAL_REMOTE_POS 16
#define HNS_RN_PHYS_RN_SRC_TYPE_POS     22
#define HNS_RN_PHYS_RN_SRC_TYPE_MASK    (0x1F)
#define HNS_RN_PHYS_RN_NODE_ID_MASK     (0x7FF)

#define HNS_RN_PHYIDS_REG0 0

/* HN-S CPA Programming */
#define HNS_RN_PHYS_CPA_GRP_RA_POS 17
#define HNS_RN_PHYS_CPA_EN_RA_POS  30
#define CPAG_TGTID_PER_GROUP       5
#define CPAG_TGTID_WIDTH           12
#define CPAG_TGTID_WIDTH_PER_GROUP 60

/* LCN Programming */
#define LCN_HASHED_TGT_GRP_TGT_TYPE_POS  2
#define LCN_HTG_REGION_BASE_ADDR_POS     16
#define LCN_HTG_REGION_END_ADDR_POS      16
#define LCN_HTG_REGION_SIZE_POS          56
#define LCN_HTG_HN_COUNT_PER_REG         8
#define LCN_HTG_HN_COUNT_REG_NUM_HN_POS  8
#define LCN_HTG_CAL_MODE_PER_REG         4
#define LCN_HTG_CAL_MODE_EN_POS          16
#define LCN_HTG_HNF_PER_CPAG_PERHNF_REG  8
#define LCN_HTG_CPAG_PER_HNF_POS         8
#define LCN_HTG_CPA_EN_HNF_COUNT_PER_REG 64

/* HN-S programming context structure */
struct cmn_cyprus_hns_ctx {
    /* Range comparison mode status flag */
    bool rcomp_en;

    /*
     * Minimum size of the regions when RCOMP is enabled. The size is
     * defined using user parameter HNSAM_RCOMP_LSB during CMN mesh build time.
     *
     * The values from 20 to 26 are valid for HNSAM_RCOMP_LSB parameter.
     * HNSAM_RCOMP_LSB = 20, defines minimum memory size = 1MB
     * HNSAM_RCOMP_LSB = 21, defines minimum memory size = 2MB
     * ...
     * HNSAM_RCOMP_LSB = 26, defines minimum memory size = 64MB
     */
    uint64_t min_region_size;

    /* Flag to indicate that the HN-S context has been initialized */
    bool is_initialized;
};

/*!
 * LCN SAM programming context structure.
 */
struct cmn_cyprus_lcnsam_ctx {
    /*! Hashed regions range comparison mode status flag */
    bool rcomp_en;

    /*!
     * Minimum size of the hashed regions when RCOMP is enabled. The size is
     * defined using user parameter RNSAM_HTG_RCOMP_LSB.
     *
     * The following values are valid:
     * RNSAM_HTG_RCOMP_LSB = 16, defines minimum memory size = 64KB
     * RNSAM_HTG_RCOMP_LSB = 17, defines minimum memory size = 128KB
     * ...
     * RNSAM_HTG_RCOMP_LSB = 26, defines minimum memory size = 64MB
     */
    uint64_t min_htg_size;

    /*!
     * LSB address mask for programming hashed target group region base address
     * and end address.
     */
    uint64_t htg_lsb_addr_mask;

    /*! Flag to indicate that the LCN SAM context has been initialized */
    bool is_initialized;
};

static struct cmn_cyprus_hns_ctx hns_ctx;
static struct cmn_cyprus_lcnsam_ctx lcnsam_ctx;

static uint8_t get_hns_rcomp_lsb_pos(struct cmn_cyprus_hns_reg *hns)
{
    return (
        (hns->UNIT_INFO[1] & HNS_UNIT_INFO_HNSAM_RCOMP_LSB_MASK) >>
        HNS_UNIT_INFO_HNSAM_RCOMP_LSB_POS);
}

static void configure_lcn_htg_region_start_and_end_addr(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    uint64_t sec_region_base,
    uint64_t sec_region_size)
{
    uint64_t lsb_addr_mask;

    lsb_addr_mask = lcnsam_ctx.htg_lsb_addr_mask;

    hns->LCN_HASHED_TGT_GRP_CFG2_REGION[region_idx] =
        (((base + size - 1) & ~lsb_addr_mask) << LCN_HTG_REGION_END_ADDR_POS);

    hns->LCN_HASHED_TGT_GRP_CFG1_REGION[region_idx] =
        ((base & ~lsb_addr_mask) << LCN_HTG_REGION_BASE_ADDR_POS);

    if (sec_region_size != 0) {
        hns->LCN_HASHED_TGT_GRP_SEC_CFG1_REGION[region_idx] =
            (sec_region_base & ~lsb_addr_mask);

        hns->LCN_HASHED_TGT_GRP_SEC_CFG2_REGION[region_idx] =
            ((sec_region_base + sec_region_size - 1) & ~lsb_addr_mask);
    }
}

/* Configurable base address & region size - Legacy CMN mode */
static void configure_lcn_htg_region_base_and_size(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    uint64_t sec_region_base,
    uint64_t sec_region_size)
{
    hns->LCN_HASHED_TGT_GRP_CFG1_REGION[region_idx] =
        sam_encode_region_size(size, lcnsam_ctx.min_htg_size)
        << LCN_HTG_REGION_SIZE_POS;

    hns->LCN_HASHED_TGT_GRP_CFG1_REGION[region_idx] |=
        (base / lcnsam_ctx.min_htg_size) << LCN_HTG_REGION_BASE_ADDR_POS;

    if (sec_region_size != 0) {
        hns->LCN_HASHED_TGT_GRP_SEC_CFG1_REGION[region_idx] |=
            sam_encode_region_size(sec_region_size, lcnsam_ctx.min_htg_size)
            << LCN_HTG_REGION_SIZE_POS;

        hns->LCN_HASHED_TGT_GRP_SEC_CFG1_REGION[region_idx] |=
            (sec_region_base / lcnsam_ctx.min_htg_size)
            << LCN_HTG_REGION_BASE_ADDR_POS;
    }
}

void hns_enable_sn_mode(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hnf_sam_hashed_mode sn_mode)
{
    /* Enable HN-F to SN-F memory striping mode */
    hns->SAM_CONTROL |= (UINT64_C(1) << HNS_SAM_CONTROL_SN_MODE_POS(sn_mode));
}

void hns_configure_top_address_bit(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hns_sam_top_address_bit top_address_bit,
    unsigned int top_address_bit_value)
{
    /* Configure bit position of top_address_bit0 */
    hns->SAM_CONTROL |=
        ((uint64_t)top_address_bit_value
         << HNS_SAM_CONTROL_TOP_ADDR_BIT_POS(top_address_bit));
}

void hns_configure_sn_node_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int sn_node_id,
    uint8_t sn_idx)
{
    /* Only 3 target SN node IDs can be programmed in a HN-S node */
    fwk_assert(sn_idx < HNS_SAM_CONTROL_SN_IDX_COUNT);

    /* Configure the SN node ID */
    hns->SAM_CONTROL |=
        ((uint64_t)sn_node_id << HNS_SAM_CONTROL_SN_NODE_ID_POS(sn_idx));
}

bool hns_is_range_comparison_mode_enabled(struct cmn_cyprus_hns_reg *hns)
{
    return (hns->UNIT_INFO[1] & HNS_UNIT_INFO_HNSAM_RCOMP_EN_MASK) >>
        HNS_UNIT_INFO_HNSAM_RCOMP_EN_POS;
}

void hns_configure_non_hashed_region_addr_range(
    struct cmn_cyprus_hns_reg *hns,
    bool hnsam_range_comp_en_mode,
    uint64_t base,
    uint64_t size,
    unsigned int non_hashed_region_idx)
{
    /* Only 2 SAM_MEMREGION registers have been defined in the driver */
    fwk_assert(non_hashed_region_idx < 2);

    /* Configure non-hashed region based on address range comparison mode */
    if (hnsam_range_comp_en_mode) {
        /* Configure end address of the region */
        hns->SAM_MEMREGION_END_ADDR[non_hashed_region_idx] =
            ((base + size - 1));

        /* Configure base address of the region */
        hns->SAM_MEMREGION[non_hashed_region_idx] |=
            ((base / hns_ctx.min_region_size) << HNS_SAM_MEMREGION_BASE_POS);
    } else {
        /* Configure region size */
        hns->SAM_MEMREGION[non_hashed_region_idx] |=
            (sam_encode_region_size(size, hns_ctx.min_region_size)
             << HNS_SAM_MEMREGION_SIZE_POS);

        /* Configure region base */
        hns->SAM_MEMREGION[non_hashed_region_idx] |=
            ((base / hns_ctx.min_region_size) << HNS_SAM_MEMREGION_BASE_POS);
    }
}

void hns_configure_non_hashed_region_sn_node_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int sn_node_id,
    unsigned int non_hashed_region_idx)
{
    /*
     * Only 2 registers have been defined in the driver for configuring
     * non-hashed memory regions.
     */
    fwk_assert(non_hashed_region_idx < 2);

    /* Clear the SN node ID */
    hns->SAM_MEMREGION[non_hashed_region_idx] &=
        ~(HNS_SAM_MEMREGION_SN_NODE_ID_MASK);

    /* Configure target node ID */
    hns->SAM_MEMREGION[non_hashed_region_idx] |= sn_node_id;
}

void hns_set_non_hashed_region_valid(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int non_hashed_region_idx)
{
    /* Only 2 SAM_MEMREGION registers have been defined in the driver */
    fwk_assert(non_hashed_region_idx < 2);

    /* Set the region as valid */
    hns->SAM_MEMREGION[non_hashed_region_idx] |= HNS_SAM_MEMREGION_VALID;
}

void hns_set_pwpr_dynamic_enable(struct cmn_cyprus_hns_reg *hns)
{
    /* Clear the dyn_en bit */
    hns->PPU_PWPR &= ~(0x1 << HNS_PWPR_DYN_EN_POS);

    /* Enable dynamic transition for system cache RAM PPU */
    hns->PPU_PWPR |= (0x1 << HNS_PWPR_DYN_EN_POS);
}

void hns_set_pwpr_op_mode(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hns_pwpr_op_mode op_mode)
{
    /* Clear the bit op_mode bits */
    hns->PPU_PWPR &= ~(0xF << HNS_PWPR_OP_MODE_POS);

    /* Configure HN-F operational power mode */
    hns->PPU_PWPR |= (op_mode << HNS_PWPR_OP_MODE_POS);
}

void hns_set_pwpr_policy(struct cmn_cyprus_hns_reg *hns, uint8_t policy)
{
    /* Clear the power policy bits */
    hns->PPU_PWPR &= ~(0xF << HNS_PWPR_POLICY_POS);

    /* Configure HN-F power policy */
    hns->PPU_PWPR |= (policy << HNS_PWPR_POLICY_POS);
}

/* Configure Node ID for Request Node (RN) and set the valid bit */
int hns_configure_rn_node_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid,
    unsigned int node_id)
{
    if (ldid >= HNS_RN_CLUSTER_MAX) {
        return FWK_E_RANGE;
    }

    /*
     * Configure the RN node ID in non-clustered mode.
     *
     * Each entry in the RN‑F vector index is associated with a single
     * RN‑F. In non-clustered mode, CMN‑Cyprus is limited to a maximum of 128
     * RN‑Fs and only nodeid_lid#{index}_ra0 bitfield is programmed in this
     * mode.
     *
     * Note: The bitfields nodeid_lid#{index}_ra1 to nodeid_lid#{index}_ra7 in
     * the registers cmn_hns_rn_cluster0-127_physid_reg[1-3] are configured only
     * in clustered mode where a maximum of 512 RN-Fs are allowed in the system.
     */
    hns->HNS_RN_CLUSTER_PHYSID[ldid][HNS_RN_PHYIDS_REG0] |=
        (node_id & HNS_RN_PHYS_RN_NODE_ID_MASK);

    /* Mark the mapping as valid */
    hns->HNS_RN_CLUSTER_PHYSID[ldid][HNS_RN_PHYIDS_REG0] |=
        (UINT64_C(0x1) << HNS_RN_PHYS_RN_ID_VALID_POS);

    return FWK_SUCCESS;
}

/* Set the Request Node (RN) as remote */
inline int hns_set_rn_node_remote(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid)
{
    if (ldid >= HNS_RN_CLUSTER_MAX) {
        return FWK_E_RANGE;
    }

    hns->HNS_RN_CLUSTER_PHYSID[ldid][HNS_RN_PHYIDS_REG0] |=
        (UINT64_C(0x1) << HNS_RN_PHYS_RN_LOCAL_REMOTE_POS);

    return FWK_SUCCESS;
}

/* Configure CHI source type for the Request Node (RN) */
inline int hns_set_rn_node_src_type(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid,
    enum cmn_cyprus_hns_rn_src_type src_type)
{
    if (ldid >= HNS_RN_CLUSTER_MAX) {
        return FWK_E_RANGE;
    }

    hns->HNS_RN_CLUSTER_PHYSID[ldid][HNS_RN_PHYIDS_REG0] |=
        ((uint64_t)(src_type & HNS_RN_PHYS_RN_SRC_TYPE_MASK)
         << HNS_RN_PHYS_RN_SRC_TYPE_POS);

    return FWK_SUCCESS;
}

/* Enable CPA mode for the Request Node (RN) */
inline int hns_enable_rn_cpa(struct cmn_cyprus_hns_reg *hns, unsigned int ldid)
{
    if (ldid >= HNS_RN_CLUSTER_MAX) {
        return FWK_E_RANGE;
    }

    hns->HNS_RN_CLUSTER_PHYSID[ldid][HNS_RN_PHYIDS_REG0] |=
        (UINT64_C(0x1) << HNS_RN_PHYS_CPA_EN_RA_POS);

    return FWK_SUCCESS;
}

/*
 * Configure target CPA Group ID for the Request Node (RN). The snoop traffic
 * targeting this RN is distributed across this CPA Group.
 */
inline int hns_configure_rn_cpag_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid,
    uint8_t cpag_id)
{
    if (ldid >= HNS_RN_CLUSTER_MAX) {
        return FWK_E_RANGE;
    }

    hns->HNS_RN_CLUSTER_PHYSID[ldid][HNS_RN_PHYIDS_REG0] |=
        (cpag_id << HNS_RN_PHYS_CPA_GRP_RA_POS);

    return FWK_SUCCESS;
}

/* Configure target node ID in the CPA Group */
int hns_configure_rn_cpag_node_id(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t cpag_tgt_idx,
    unsigned int ccg_ha_node_id)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    register_idx = (cpag_tgt_idx / CPAG_TGTID_PER_GROUP);
    if (register_idx >= HNS_CPA_GRP_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos = ((cpag_tgt_idx * CPAG_TGTID_WIDTH) % CPAG_TGTID_WIDTH_PER_GROUP);
    hns->CML_PORT_AGGR_GRP_REG[register_idx] |= ((uint64_t)ccg_ha_node_id)
        << bit_pos;

    return FWK_SUCCESS;
}

/* Configure the HTG region range in LCN SAM */
int hns_configure_lcn_htg_region_range(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    const struct mod_cmn_cyprus_mem_region_map *region)
{
    uint64_t base;
    uint64_t size;
    uint64_t sec_region_base;
    uint64_t sec_region_size;

    base = region->base;
    size = region->size;
    sec_region_base = region->sec_region_base;
    sec_region_size = region->sec_region_size;

    if (lcnsam_ctx.rcomp_en == true) {
        /* Configure start and end address of the region */
        configure_lcn_htg_region_start_and_end_addr(
            hns, region_idx, base, size, sec_region_base, sec_region_size);
    } else {
        /* Configure base address and size of the region */
        configure_lcn_htg_region_base_and_size(
            hns, region_idx, base, size, sec_region_base, sec_region_size);
    }

    return FWK_SUCCESS;
}

/* Set the target type for the HTG region in LCN SAM */
void hns_set_lcn_htg_region_target_type(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    enum lcn_sam_node_type target_type)
{
    /* Set target type for primary region */
    hns->LCN_HASHED_TGT_GRP_CFG1_REGION[region_idx] |=
        (target_type << LCN_HASHED_TGT_GRP_TGT_TYPE_POS);

    /* Set target type for secondary region */
    hns->LCN_HASHED_TGT_GRP_SEC_CFG1_REGION[region_idx] |=
        (target_type << LCN_HASHED_TGT_GRP_TGT_TYPE_POS);
}

/* Configure the target node count for the HTG region in LCN SAM */
void hns_set_lcn_htg_region_hn_count(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    unsigned int hn_count)
{
    uint32_t register_idx;
    uint8_t bit_pos;

    register_idx = (region_idx / LCN_HTG_HN_COUNT_PER_REG);
    bit_pos = (region_idx * LCN_HTG_HN_COUNT_REG_NUM_HN_POS);

    hns->LCN_HASHED_TARGET_GROUP_HN_COUNT_REG[register_idx] |=
        (hn_count << bit_pos);
}

/* Enable CAL mode for the HTG region in LCN SAM */
void hns_enable_lcn_htg_cal_mode(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx)
{
    uint32_t register_idx;
    uint8_t bit_pos;

    register_idx = (region_idx / LCN_HTG_CAL_MODE_PER_REG);
    bit_pos = (region_idx * LCN_HTG_CAL_MODE_EN_POS);

    hns->LCN_HASHED_TARGET_GRP_CAL_MODE_REG[register_idx] |=
        (UINT64_C(0x1) << bit_pos);
}

/* Enable CPA mode for the HTG region in LCN SAM */
int hns_enable_lcn_htg_cpa_mode(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    unsigned int hns_count)
{
    uint32_t register_idx;
    uint64_t mask;
    uint8_t start_bit_pos, end_bit_pos;

    start_bit_pos =
        ((region_idx * hns_count) % LCN_HTG_CPA_EN_HNF_COUNT_PER_REG);

    end_bit_pos = ((start_bit_pos + hns_count) - 1);

    /* Check if the mask fits within the register width */
    if (end_bit_pos >= LCN_HTG_CPA_EN_HNF_COUNT_PER_REG) {
        return FWK_E_RANGE;
    }

    /*
     * The following calculation is based on the assumption that there's one
     * local SCG and the remote SCG's HN-S count is equal to that of the local
     * SCG when LCN is enabled in CML configurations.
     */
    register_idx =
        ((region_idx * hns_count) / LCN_HTG_CPA_EN_HNF_COUNT_PER_REG);

    /* Create a mask for enabling the CPA for each HN in the SCG */
    mask = GET_BIT_MASK(hns_count);

    /* Enable CPA mode for remote HN-S */
    hns->LCN_HASHED_TARGET_GRP_HNF_CPA_EN_REG[register_idx] |=
        (mask << start_bit_pos);

    return FWK_SUCCESS;
}

/* Configure the target CPA Group ID for the HTG region in LCN SAM */
void hns_set_lcn_htg_cpag_id(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t region_idx,
    unsigned int hns_count,
    uint8_t cpag_id)
{
    unsigned int hns_idx;
    unsigned int hns_idx_start;
    unsigned int hns_idx_end;
    uint32_t register_idx;
    uint8_t bit_pos;

    hns_idx_start = region_idx * hns_count;
    hns_idx_end = hns_idx_start + hns_count;

    for (hns_idx = hns_idx_start; hns_idx < hns_idx_end; hns_idx++) {
        register_idx = (hns_idx / LCN_HTG_HNF_PER_CPAG_PERHNF_REG);
        bit_pos =
            ((hns_idx % LCN_HTG_HNF_PER_CPAG_PERHNF_REG) *
             LCN_HTG_CPAG_PER_HNF_POS);

        hns->LCN_HASHED_TARGET_GRP_CPAG_PERHNF_REG[register_idx] |=
            (cpag_id << bit_pos);
    }
}

/* Set the HTG region as valid in LCN SAM */
void hns_set_lcn_htg_region_valid(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t region_idx)
{
    hns->LCN_HASHED_TGT_GRP_CFG1_REGION[region_idx] |= 0x1;
}

/* Set the secondary HTG region as valid in LCN SAM */
void hns_set_lcn_htg_sec_region_valid(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t region_idx)
{
    hns->LCN_HASHED_TGT_GRP_SEC_CFG1_REGION[region_idx] |= 0x1;
}

int setup_lcnsam_ctx(struct cmn_cyprus_rnsam_reg *rnsam)
{
    if (lcnsam_ctx.is_initialized) {
        return FWK_SUCCESS;
    }

    if (rnsam == NULL) {
        return FWK_E_PARAM;
    }

    /*
     * Check if the start and end address has to be programmed for
     * HTG regions.
     */
    lcnsam_ctx.rcomp_en = rnsam_get_htg_rcomp_en_mode(rnsam);

    /* Get minimum region size for HTG regions */
    lcnsam_ctx.min_htg_size = rnsam_get_htg_rcomp_en_mode(rnsam);

    /* Get the LSB mask from LSB bit position defining minimum region size */
    lcnsam_ctx.htg_lsb_addr_mask = rnsam_get_htg_lsb_addr_mask(rnsam);

    lcnsam_ctx.is_initialized = true;

    return FWK_SUCCESS;
}

int setup_hns_ctx(struct cmn_cyprus_hns_reg *hns)
{
    uint8_t lsb_pos;

    /* Return if initialized already */
    if (hns_ctx.is_initialized == true) {
        return FWK_SUCCESS;
    }

    if (hns == NULL) {
        return FWK_E_PARAM;
    }

    /*
     * Check if the start and end address has to be programmed for
     * regions.
     */
    hns_ctx.rcomp_en = hns_is_range_comparison_mode_enabled(hns);

    /* Get the minimum region size */
    if (hns_ctx.rcomp_en != 0) {
        lsb_pos = get_hns_rcomp_lsb_pos(hns);
        hns_ctx.min_region_size = fwk_math_pow2(lsb_pos);
    } else {
        hns_ctx.min_region_size = HNSAM_REGION_MIN_SIZE;
    }

    hns_ctx.is_initialized = true;

    return FWK_SUCCESS;
}

/*
 * Helper function to check if the given HN-S node pointer points to an isolated
 * HN-S node. Isolated HN-S nodes are skipped during the CMN Discovery and hence
 * point to null.
 */
inline bool is_hns_node_isolated(struct cmn_cyprus_hns_reg *hns)
{
    return (hns == NULL);
}
