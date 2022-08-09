/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn700.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_math.h>

#include <inttypes.h>

#define MOD_NAME "[CMN700] "

/*
 * Encoding bits size of the X and Y position in the Node info value.
 * If X and Y dimension are less than 4, encoding bits size will be 2.
 * If X and Y dimension are between 5 and 8, encoding bits size will be 3.
 */
static unsigned int encoding_bits;
static unsigned int mask_bits;

static const char *const cmn700_rev_to_name[] = {
    [CMN700_PERIPH_ID_2_REV_R0_P0] = "r0p0",
    [CMN700_PERIPH_ID_2_REV_R1_P0] = "r1p0",
    [CMN700_PERIPH_ID_2_REV_R1_P1] = "r1p1",
    [CMN700_PERIPH_ID_2_REV_R2_P0] = "r2p0",
    [CMN700_PERIPH_ID_UNKNOWN_REV] = "Unknown!",
};

unsigned int get_node_device_port_count(void *node_base)
{
    struct node_header *node = node_base;
    return (node->NODE_INFO & CMN700_MXP_NODE_INFO_NUM_DEVICE_PORT_MASK) >>
        CMN700_MXP_NODE_INFO_NUM_DEVICE_PORT_POS;
}

unsigned int get_node_child_count(void *node_base)
{
    struct node_header *node = node_base;
    return node->CHILD_INFO & CMN700_CHILD_INFO_COUNT;
}

enum node_type get_node_type(void *node_base)
{
    struct node_header *node = node_base;
    return (enum node_type)(node->NODE_INFO & CMN700_NODE_INFO_TYPE);
}

unsigned int get_node_id(void *node_base)
{
    struct node_header *node = node_base;
    return (node->NODE_INFO & CMN700_NODE_INFO_ID) >> CMN700_NODE_INFO_ID_POS;
}

unsigned int get_node_logical_id(void *node_base)
{
    struct node_header *node = node_base;
    return (node->NODE_INFO & CMN700_NODE_INFO_LOGICAL_ID) >>
        CMN700_NODE_INFO_LOGICAL_ID_POS;
}

void *get_child_node(uintptr_t base, void *node_base, unsigned int child_index)
{
    struct node_header *node = node_base;
    uint32_t child_pointer;
    unsigned int offset;
    void *child_node;

    child_pointer = node->CHILD_POINTER[child_index];
    offset = child_pointer & CMN700_CHILD_POINTER_OFFSET;

    child_node = (void *)(base + offset);
    return child_node;
}

unsigned int get_child_node_id(void *node_base, unsigned int child_index)
{
    struct node_header *node = node_base;
    uint32_t node_pointer;
    unsigned int node_id;

    node_pointer = (node->CHILD_POINTER[child_index] &
                    CMN700_CHILD_POINTER_EXT_NODE_POINTER) >>
        CMN700_CHILD_POINTER_EXT_NODE_POINTER_POS;

    /*
     * For mesh widths using 2 bits each for X,Y encoding:
     * NodeID[1:0] = DeviceID[3:2]
     * NodeID[2]   = DeviceID[0]
     * NodeID[4:3] = NODE POINTER[7:6]
     * NodeID[6:5] = NODE POINTER[9:8]
     *
     * For mesh widths using 3 bits each for X,Y encoding:
     * NodeID[1:0] = DeviceID[3:2]
     * NodeID[2]   = DeviceID[0]
     * NodeID[5:3] = NODE POINTER[8:6]
     * NodeID[8:6] = NODE POINTER[11:9]
     */
    node_id = (((node_pointer >> 6) & 0xff) << 3) |
        ((node_pointer & 0x1) << 2) | ((node_pointer >> 2) & 0x3);

    return node_id;
}

unsigned int get_cmn700_revision(struct cmn700_cfgm_reg *root)
{
    return (root->PERIPH_ID[1] & CMN700_PERIPH_ID_2_MASK) >>
        CMN700_PERIPH_ID_2_REV_POS;
}

const char *get_cmn700_revision_name(struct cmn700_cfgm_reg *root)
{
    unsigned int revision;

    revision = get_cmn700_revision(root);
    if (revision > CMN700_PERIPH_ID_UNKNOWN_REV)
        revision = CMN700_PERIPH_ID_UNKNOWN_REV;

    return cmn700_rev_to_name[revision];
}

bool is_child_external(void *node_base, unsigned int child_index)
{
    struct node_header *node = node_base;
    return !!(node->CHILD_POINTER[child_index] & (1U << 31));
}

unsigned int get_port_number(
    unsigned int child_node_id,
    unsigned int xp_port_cnt)
{
    if (xp_port_cnt == 2) {
        return (child_node_id >> CMN700_NODE_ID_PORT_POS) &
            CMN700_NODE_ID_PORT_MASK;
    } else {
        /* For port counts 3 and 4 */
        return (child_node_id >> CMN700_MULTI_PORTS_NODE_ID_PORT_POS) &
            CMN700_MULTI_PORTS_NODE_ID_PORT_MASK;
    }
}

unsigned int get_device_type(void *mxp_base, int port)
{
    struct cmn700_mxp_reg *mxp = mxp_base;
    return mxp->PORT_CONNECT_INFO[port] &
        CMN700_MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK;
}

bool is_cal_connected(void *mxp_base, bool port)
{
    struct cmn700_mxp_reg *mxp = mxp_base;
    return (mxp->PORT_CONNECT_INFO[port] &
            CMN700_MXP_PORT_CONNECT_INFO_CAL_CONNECTED_MASK) >>
        CMN700_MXP_PORT_CONNECT_INFO_CAL_CONNECTED_POS;
}

bool is_device_type_rnf(void *mxp_base, bool port)
{
    return (
        (get_device_type(mxp_base, port) == DEVICE_TYPE_RN_F_CHIB_ESAM) ||
        (get_device_type(mxp_base, port) == DEVICE_TYPE_RN_F_CHIC_ESAM) ||
        (get_device_type(mxp_base, port) == DEVICE_TYPE_RN_F_CHID_ESAM) ||
        (get_device_type(mxp_base, port) == DEVICE_TYPE_RN_F_CHIE_ESAM));
}

static unsigned int get_rnsam_htg_rcomp_lsb_bit_pos(void *rnsam_reg)
{
    struct cmn700_rnsam_reg *rnsam = rnsam_reg;
    return (
        rnsam->UNIT_INFO[1] & CMN700_RNSAM_UNIT_INFO_HTG_RCOMP_LSB_PARAM_MASK);
}

static unsigned int get_rnsam_nonhash_rcomp_lsb_bit_pos(void *rnsam_reg)
{
    struct cmn700_rnsam_reg *rnsam = rnsam_reg;
    return (rnsam->UNIT_INFO[1] &
            CMN700_RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_MASK) >>
        CMN700_RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_POS;
}

static uint64_t get_rnsam_lsb_addr_mask(void *rnsam_reg, enum sam_type sam_type)
{
    uint64_t lsb_bit_pos;
    struct cmn700_rnsam_reg *rnsam = rnsam_reg;

    lsb_bit_pos = (sam_type == SAM_TYPE_NON_HASH_MEM_REGION) ?
        get_rnsam_nonhash_rcomp_lsb_bit_pos(rnsam) :
        get_rnsam_htg_rcomp_lsb_bit_pos(rnsam);

    return (1 << lsb_bit_pos) - 1;
}

bool get_rnsam_nonhash_range_comp_en_mode(void *rnsam_reg)
{
    struct cmn700_rnsam_reg *rnsam = rnsam_reg;
    return (rnsam->UNIT_INFO[0] &
            CMN700_RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_MASK) >>
        CMN700_RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_POS;
}

bool get_rnsam_htg_range_comp_en_mode(void *rnsam_reg)
{
    struct cmn700_rnsam_reg *rnsam = rnsam_reg;
    return (rnsam->UNIT_INFO[0] &
            CMN700_RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_MASK) >>
        CMN700_RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_POS;
}

bool get_hnsam_range_comp_en_mode(void *hnf_reg)
{
    struct cmn700_hnf_reg *hnf = hnf_reg;
    return (hnf->UNIT_INFO[1] & CMN700_HNF_UNIT_INFO_HNSAM_RCOMP_EN_MASK) >>
        CMN700_HNF_UNIT_INFO_HNSAM_RCOMP_EN_POS;
}

uint64_t sam_encode_region_size(uint64_t size)
{
    uint64_t blocks;
    uint64_t result;

    /* Size must be a multiple of SAM_GRANULARITY */
    fwk_assert((size % SAM_GRANULARITY) == 0);

    /* Size also must be a power of two */
    fwk_assert((size & (size - 1)) == 0);

    blocks = size / SAM_GRANULARITY;
    result = fwk_math_log2(blocks);

    return result;
}

bool is_region_aligned(
    struct cmn700_rnsam_reg *rnsam,
    struct mod_cmn700_mem_region_map *mmap,
    enum sam_type sam_type)
{
    uint64_t lsb_addr_mask;

    lsb_addr_mask = get_rnsam_lsb_addr_mask(rnsam, sam_type);
    return ((mmap->base & ~lsb_addr_mask) & (mmap->size & ~lsb_addr_mask)) == 0;
}

bool is_non_hash_region_mapped(
    struct cmn700_rnsam_reg *rnsam,
    uint32_t region_io_count,
    struct mod_cmn700_mem_region_map *mmap,
    uint32_t *region_index)
{
    int idx;
    unsigned int programmed_node_id;
    uint32_t group;
    uint32_t bit_pos;
    volatile uint64_t *reg;
    uint64_t lsb_addr_mask;

    lsb_addr_mask =
        get_rnsam_lsb_addr_mask(rnsam, SAM_TYPE_NON_HASH_MEM_REGION);

    for (idx = region_io_count - 1; idx >= 0; idx--) {
        reg = &rnsam->NON_HASH_MEM_REGION[idx];
        if (mmap->base == (*reg & ~lsb_addr_mask)) {
            group = idx / CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;
            bit_pos = CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
                (idx % CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);
            programmed_node_id =
                (rnsam->NON_HASH_TGT_NODEID[group] >> bit_pos) &
                CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK;
            mmap->node_id &= CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK;

            if (programmed_node_id == mmap->node_id) {
                FWK_LOG_INFO(
                    MOD_NAME "Found region: %d mapped for Node: %u ",
                    idx,
                    mmap->node_id);
                *region_index = idx;

                return true;
            } else {
                FWK_LOG_ERR(
                    MOD_NAME
                    "Address: 0x%llx mapped to different node id:"
                    " %u than expected: %u\n",
                    mmap->base,
                    programmed_node_id,
                    mmap->node_id);
                fwk_unexpected();
            }
        }
    }

    return false;
}

void configure_region(
    void *rnsam_reg,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type node_type,
    enum sam_type sam_type)
{
    bool prog_start_and_end_addr;
    uint64_t lsb_addr_mask;
    uint64_t value;
    struct cmn700_rnsam_reg *rnsam = rnsam_reg;
    volatile uint64_t *reg;
    volatile uint64_t *reg_cfg2;

    fwk_assert(rnsam_reg);

    if (sam_type == SAM_TYPE_NON_HASH_MEM_REGION) {
        if (region_idx >= MAX_NON_HASH_MEM_COUNT) {
            FWK_LOG_ERR(
                MOD_NAME
                "Region idx: %d should be less than max non hash region count "
                "of %d",
                region_idx,
                MAX_NON_HASH_MEM_COUNT);
            fwk_unexpected();
        }

        if (region_idx < NON_HASH_MEM_REG_COUNT) {
            reg = &rnsam->NON_HASH_MEM_REGION[region_idx];
            reg_cfg2 = &rnsam->NON_HASH_MEM_REGION_CFG2[region_idx];
        } else {
            reg = &rnsam->NON_HASH_MEM_REGION_GRP2
                       [region_idx - NON_HASH_MEM_REG_COUNT];
            reg_cfg2 = &rnsam->NON_HASH_MEM_REGION_CFG2_GRP2
                            [region_idx - NON_HASH_MEM_REG_COUNT];
        }
    } else if (sam_type == SAM_TYPE_SYS_CACHE_GRP_REGION) {
        if (region_idx >= MAX_SCG_COUNT) {
            FWK_LOG_ERR(
                MOD_NAME
                "Region idx: %d should be less then max SCG region count of %d",
                region_idx,
                MAX_SCG_COUNT);
            fwk_unexpected();
        }
        reg = &rnsam->SYS_CACHE_GRP_REGION[region_idx];
        reg_cfg2 = &rnsam->HASHED_TGT_GRP_CFG2_REGION[region_idx];
    } else {
        FWK_LOG_ERR(MOD_NAME "Unexpected sam_type!");
        fwk_unexpected();
        return;
    }

    /* Check if the start and end address has to be programmed */
    prog_start_and_end_addr = (sam_type == SAM_TYPE_NON_HASH_MEM_REGION) ?
        get_rnsam_nonhash_range_comp_en_mode(rnsam) :
        get_rnsam_htg_range_comp_en_mode(rnsam);

    if ((!prog_start_and_end_addr) && ((base % size) != 0)) {
        FWK_LOG_ERR(
            MOD_NAME "Base: 0x%" PRIx64 " should align with Size: 0x%" PRIx64,
            base,
            size);
        fwk_unexpected();
    }

    /* Get the LSB mask from LSB bit position defining minimum region size */
    lsb_addr_mask = get_rnsam_lsb_addr_mask(rnsam, sam_type);

    value = CMN700_RNSAM_REGION_ENTRY_VALID;
    value |= node_type << CMN700_RNSAM_REGION_ENTRY_TYPE_POS;

    if (prog_start_and_end_addr) {
        value |= (base & ~lsb_addr_mask);
        *reg = value;
        *reg_cfg2 = (base + size - 1) & ~lsb_addr_mask;
    } else {
        value |= sam_encode_region_size(size)
            << CMN700_RNSAM_REGION_ENTRY_SIZE_POS;
        value |= (base / SAM_GRANULARITY) << CMN700_RNSAM_REGION_ENTRY_BASE_POS;
        *reg = value;
    }
}

static const char *const type_to_name[] = {
    [NODE_TYPE_INVALID]     = "<Invalid>",
    [NODE_TYPE_DVM]         = "DVM",
    [NODE_TYPE_CFG]         = "CFG",
    [NODE_TYPE_DTC]         = "DTC",
    [NODE_TYPE_HN_I]        = "HN-I",
    [NODE_TYPE_HN_F]        = "HN-F",
    [NODE_TYPE_XP]          = "XP",
    [NODE_TYPE_SBSX]        = "SBSX",
    [NODE_TYPE_MPAM_S]      = "MPAM-S",
    [NODE_TYPE_MPAM_NS]     = "MPAM-NS",
    [NODE_TYPE_RN_I]        = "RN-I",
    [NODE_TYPE_RN_D]        = "RN-D",
    [NODE_TYPE_RN_SAM]      = "RN-SAM",
    [NODE_TYPE_HN_P]        = "HN-P",
};

static const char *const type_to_name_cml[] = {
    [NODE_TYPE_CXRA - NODE_TYPE_CML_BASE] = "CXRA",
    [NODE_TYPE_CXHA - NODE_TYPE_CML_BASE] = "CXHA",
    [NODE_TYPE_CXLA - NODE_TYPE_CML_BASE] = "CXLA",
    [NODE_TYPE_CCRA - NODE_TYPE_CML_BASE] = "CCRA",
    [NODE_TYPE_CCHA - NODE_TYPE_CML_BASE] = "CCHA",
    [NODE_TYPE_CCLA - NODE_TYPE_CML_BASE] = "CCLA",
};

const char *get_node_type_name(enum node_type node_type)
{
    /* Base node IDs */
    if (node_type <= NODE_TYPE_HN_P)
        return type_to_name[node_type];

    /* CML node IDs */
    if ((node_type >= NODE_TYPE_CML_BASE) && (node_type <= NODE_TYPE_CCLA))
        return type_to_name_cml[node_type - NODE_TYPE_CML_BASE];

    /* Invalid node IDs */
    return type_to_name[NODE_TYPE_INVALID];
}

unsigned int get_node_pos_x(void *node_base)
{
    struct node_header *node = node_base;
    return (get_node_id(node) >> (CMN700_NODE_ID_Y_POS + encoding_bits)) &
        mask_bits;
}

unsigned int get_node_pos_y(void *node_base)
{
    struct node_header *node = node_base;
    return (get_node_id(node) >> CMN700_NODE_ID_Y_POS) & mask_bits;
}

void set_encoding_and_masking_bits(const struct mod_cmn700_config *config)
{
    unsigned int mesh_size_x;
    unsigned int mesh_size_y;

    mesh_size_x = config->mesh_size_x;
    mesh_size_y = config->mesh_size_y;

    /*
     * Determine the number of bits used to represent each node coordinate based
     * on the mesh size as per CMN700 specification.
     */
    if ((mesh_size_x > 8) || (mesh_size_y > 8)) {
        encoding_bits = 4;
    } else {
        encoding_bits = ((mesh_size_x > 4) || (mesh_size_y > 4)) ? 3 : 2;
    }

    /* Extract node coordinates from the node identifier */

    mask_bits = (1 << encoding_bits) - 1;
}
