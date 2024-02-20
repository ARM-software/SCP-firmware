/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing RNSAM register.
 */

#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_reg.h>
#include <internal/cmn_cyprus_rnsam_reg.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_math.h>

#include <stdbool.h>
#include <stdint.h>

/* RNSAM STALL/UNSTALL */
#define RNSAM_STATUS_UNSTALL               UINT64_C(0x02)
#define RNSAM_STATUS_USE_DEFAULT_TARGET_ID UINT64_C(0x01)

/* RNSAM non-hashed region range comparison */
#define RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_MASK UINT64_C(0x80000000)
#define RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_POS  31

/* RNSAM HTG range comparison */
#define RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_MASK UINT64_C(0x8000000)
#define RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_POS  27

/* RNSAM RCOMP LSB */
#define RNSAM_UNIT_INFO_HTG_RCOMP_LSB_PARAM_MASK     UINT64_C(0x1F)
#define RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_MASK UINT64_C(0x3E0)
#define RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_POS  5

/* RNSAM memory region entry */
#define RNSAM_REGION_ENTRY_VALID           UINT64_C(0x01)
#define RNSAM_REGION_ENTRY_TARGET_TYPE_POS 2
#define RNSAM_REGION_ENTRY_SIZE_POS        56
#define RNSAM_REGION_ENTRY_BASE_POS        26
#define RNSAM_ENCODED_REGION_SIZE_MASK     UINT64_C(0x7F)

/* RNSAM Non-hashed region target node id */
#define RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH  12
#define RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP 4

/* RNSAM SCG/HTG target node ID */
#define RNSAM_SCG_TARGET_ENTRIES_PER_GROUP 4
#define RNSAM_SCG_TARGET_ENTRY_BITS_WIDTH  12
#define RNSAM_SCG_HN_CNT_POS(scg_grp)      (8 * (scg_grp))

/* RNSAM CAL Mode */
#define RNSAM_SCG_HNS_CAL_MODE_EN    UINT64_C(0x01)
#define RNSAM_SCG_HNS_CAL_MODE_SHIFT 16

/* RNSAM Hierarchical hashing */
#define RNSAM_HIERARCHICAL_HASH_EN_POS         2
#define RNSAM_HIERARCHICAL_HASH_EN_MASK        UINT64_C(0x01)
#define RNSAM_HIER_ENABLE_ADDRESS_STRIPING_POS 3
#define RNSAM_HIER_HASH_CLUSTERS_POS           8
#define RNSAM_HIER_HASH_NODES_POS              16

/*
 * Minimum size of hashed and non-hashed regions when RCOMP mode
 * is disabled is 64MB. In this mode, the base and the size of the
 * programmed region is used for comparision.
 */
#define RNSAM_REGION_MIN_SIZE (64 * FWK_MIB)

/* RNSAM CPA Programming */
#define CPA_MODE_CTRL_REGIONS_PER_GROUP    10
#define CPA_MODE_CTRL_PAG_WIDTH_PER_REGION 6
#define CPA_MODE_CTRL_PAG_GRPID_OFFSET     1
#define CPA_MODE_CTRL_PAG_GRPID_MASK       0x1F
#define CPA_CTRL_CPAG_PER_GROUP            5
#define CPA_CTRL_NUM_CXG_PAG_WIDTH         12
#define CPA_CTRL_NUM_CXG_PAG_MASK          UINT64_C(0x07)
#define CPA_CTRL_PORT_TYPE_POS             5
#define CPAG_TGTID_PER_GROUP               5
#define CPAG_TGTID_WIDTH                   12
#define CPAG_TGTID_MASK                    UINT64_C(0x7FF)
#define CPAG_TGTID_WIDTH_PER_GROUP         60
#define CPAG_BASE_INDX_WIDTH_PER_CPAG      8
#define CPAG_BASE_INDX_CPAG_PER_GROUP      8
#define CPAG_BASE_INDX_MASK                UINT64_C(0x3F)

#define MAX_CCG_COUNT_PER_CPAG 32

/* 3 CCG ports in a CPAG (MOD-3 hash) */
#define CPAG_CCG_COUNT_3 3
/* NUM_CXG_PAGx bitfield value for CPAG with 3 CCG ports (MOD-3 hash) */
#define CPAG_CCG_COUNT_VAL_MOD_3_HASH 6

/*!
 * RNSAM programming context structure.
 */
struct cmn_cyprus_rnsam_ctx {
    /*! Non-hashed regions range comparison mode status flag */
    bool non_hash_rcomp_en;

    /*!
     * LSB address mask for programming non-hashed region base address and
     * end address.
     */
    uint64_t non_hash_lsb_addr_mask;

    /*!
     * Minimum size of the Non-hashed regions when RCOMP is enabled. The size is
     * defined using user parameter RNSAM_NONHASH_RCOMP_LSB during CMN mesh
     * build time.
     *
     * The values from 16 to 26 are valid for RNSAM_NONHASH_RCOMP_LSB parameter.
     * RNSAM_NONHASH_RCOMP_LSB = 16, defines minimum memory size = 64KB
     * RNSAM_NONHASH_RCOMP_LSB = 17, defines minimum memory size = 128KB
     * ...
     * RNSAM_NONHASH_RCOMP_LSB = 26, defines minimum memory size = 64MB
     */
    uint64_t min_non_hash_size;

    /*! Hashed regions range comparison mode status flag */
    bool htg_rcomp_en;

    /*!
     * LSB address mask for programming hashed target group region base address
     * and end address.
     */
    uint64_t htg_lsb_addr_mask;

    /*!
     * Minimum size of the hashed regions when RCOMP is enabled. The size is
     * defined using user parameter RNSAM_HTG_RCOMP_LSB during CMN mesh build
     * time.
     *
     * The values from 16 to 26 are valid for RNSAM_HTG_RCOMP_LSB parameter.
     * RNSAM_HTG_RCOMP_LSB = 16, defines minimum memory size = 64KB
     * RNSAM_HTG_RCOMP_LSB = 17, defines minimum memory size = 128KB
     * ...
     * RNSAM_HTG_RCOMP_LSB = 26, defines minimum memory size = 64MB
     */
    uint64_t min_htg_size;

    /*! Flag to indicate that the RNSAM context has been initialized */
    bool is_initialized;
};

static struct cmn_cyprus_rnsam_ctx rnsam_ctx;

/*
 * Helper function to check if the number of ccg nodes per CPAG is one of the
 * following values: 1, 2, 4, 8, 16, 32 and 3 (MOD-3 hash).
 */
static bool is_cpag_ccg_count_valid(uint8_t ccg_count)
{
    if (ccg_count == CPAG_CCG_COUNT_3) {
        return true;
    }

    if (ccg_count > MAX_CCG_COUNT_PER_CPAG) {
        return false;
    }

    return (IS_POW_OF_TWO(ccg_count));
}

/*
 * Helper function to get the value to be set in the NUM_CXG_PAGx bitfield for a
 * given CPAG CCG count.
 */
static inline uint8_t get_num_cxg_value(uint8_t ccg_count)
{
    if (ccg_count == CPAG_CCG_COUNT_3) {
        return CPAG_CCG_COUNT_VAL_MOD_3_HASH;
    }

    return fwk_math_log2((unsigned int)ccg_count);
}

static bool is_non_hash_range_comparison_mode_enabled(
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    return (rnsam->UNIT_INFO[0] & RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_MASK) >>
        RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_POS;
}

static uint8_t get_rnsam_non_hash_rcomp_lsb_bit_pos(
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    return (rnsam->UNIT_INFO[1] &
            RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_MASK) >>
        RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_POS;
}

static uint64_t get_rnsam_non_hash_lsb_addr_mask(
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    uint8_t lsb_bit_pos;

    lsb_bit_pos = get_rnsam_non_hash_rcomp_lsb_bit_pos(rnsam);

    return ((1 << lsb_bit_pos) - 1);
}

static bool is_htg_range_comparison_mode_enabled(
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    return (rnsam->UNIT_INFO[0] & RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_MASK) >>
        RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_POS;
}

static uint8_t get_rnsam_htg_rcomp_lsb_bit_pos(
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    return (rnsam->UNIT_INFO[1] & RNSAM_UNIT_INFO_HTG_RCOMP_LSB_PARAM_MASK);
}

static uint64_t get_rnsam_htg_lsb_addr_mask(struct cmn_cyprus_rnsam_reg *rnsam)
{
    uint8_t lsb_bit_pos;

    lsb_bit_pos = get_rnsam_htg_rcomp_lsb_bit_pos(rnsam);

    return (1 << lsb_bit_pos) - 1;
}

static uint64_t get_min_non_hash_region_size(struct cmn_cyprus_rnsam_reg *rnsam)
{
    uint64_t min_region_size;
    uint8_t lsb_bit_pos;

    if (is_non_hash_range_comparison_mode_enabled(rnsam) == true) {
        lsb_bit_pos = get_rnsam_non_hash_rcomp_lsb_bit_pos(rnsam);
        min_region_size = fwk_math_pow2(lsb_bit_pos);
    } else {
        min_region_size = RNSAM_REGION_MIN_SIZE;
    }

    return min_region_size;
}

static uint64_t get_min_htg_size(struct cmn_cyprus_rnsam_reg *rnsam)
{
    uint64_t min_region_size;
    uint8_t lsb_bit_pos;

    if (is_htg_range_comparison_mode_enabled(rnsam) == true) {
        lsb_bit_pos = get_rnsam_htg_rcomp_lsb_bit_pos(rnsam);
        min_region_size = fwk_math_pow2(lsb_bit_pos);
    } else {
        min_region_size = RNSAM_REGION_MIN_SIZE;
    }

    return min_region_size;
}

static void set_non_hash_region_target_type(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    enum sam_node_type target_type)
{
    uint32_t register_idx;

    /* Configure target node type */
    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        rnsam->NON_HASH_MEM_REGION[region_idx] |=
            (target_type << RNSAM_REGION_ENTRY_TARGET_TYPE_POS);
    } else {
        register_idx = (region_idx - RNSAM_NON_HASH_REG_COUNT);
        rnsam->NON_HASH_MEM_REGION_GRP2[register_idx] |=
            (target_type << RNSAM_REGION_ENTRY_TARGET_TYPE_POS);
    }
}

static void set_htg_region_target_type(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    enum sam_node_type target_type)
{
    if (region_idx < RNSAM_HTG_REG_COUNT) {
        rnsam->SYS_CACHE_GRP_REGION[region_idx] |=
            (target_type << RNSAM_REGION_ENTRY_TARGET_TYPE_POS);

    } else {
        rnsam->HASHED_TGT_GRP_CFG1_REGION[region_idx - RNSAM_HTG_REG_COUNT] |=
            (target_type << RNSAM_REGION_ENTRY_TARGET_TYPE_POS);
    }
}

/* Configurable lower address & upper address */
static void configure_non_hash_region_start_and_end_addr(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size)
{
    uint64_t lsb_addr_mask;

    lsb_addr_mask = rnsam_ctx.non_hash_lsb_addr_mask;

    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        /* Configure the bits [51:RCOMP_LSB] of base address of the range */
        rnsam->NON_HASH_MEM_REGION[region_idx] = (base & ~lsb_addr_mask);

        /* Configure the end address of the region */
        rnsam->NON_HASH_MEM_REGION_CFG2[region_idx] =
            ((base + size - 1) & ~lsb_addr_mask);
    } else {
        /* Configure the bits [51:RCOMP_LSB] of base address of the range */
        rnsam->NON_HASH_MEM_REGION_GRP2[region_idx - RNSAM_NON_HASH_REG_COUNT] =
            (base & ~lsb_addr_mask);

        /* Configure the end address of the region */
        rnsam->NON_HASH_MEM_REGION_CFG2_GRP2
            [region_idx - RNSAM_NON_HASH_REG_COUNT] =
            ((base + size - 1) & ~lsb_addr_mask);
    }
}

/* Configurable base address & region size - Legacy CMN mode */
static int configure_non_hash_region_base_and_size(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size)
{
    uint8_t idx;
    uint64_t min_size;

    if ((base % size) != 0) {
        FWK_LOG_ERR(MOD_NAME "Invalid non-hashed region %u", region_idx);
        FWK_LOG_ERR(
            MOD_NAME "Base: 0x%llx should align with Size: 0x%llx", base, size);
        return FWK_E_PARAM;
    }

    min_size = rnsam_ctx.min_non_hash_size;

    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        idx = region_idx;
        /* Configure the memory region size */
        rnsam->NON_HASH_MEM_REGION[idx] = sam_encode_region_size(size, min_size)
            << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->NON_HASH_MEM_REGION[idx] |= (base / min_size)
            << RNSAM_REGION_ENTRY_BASE_POS;
    } else {
        idx = (region_idx - RNSAM_NON_HASH_REG_COUNT);
        /* Configure the memory region size */
        rnsam->NON_HASH_MEM_REGION_GRP2[idx] =
            sam_encode_region_size(size, min_size)
            << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->NON_HASH_MEM_REGION_GRP2[idx] |= (base / min_size)
            << RNSAM_REGION_ENTRY_BASE_POS;
    }

    return FWK_SUCCESS;
}

/* Configurable lower address & upper address */
static void configure_htg_region_start_and_end_addr(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size)
{
    uint64_t lsb_addr_mask;

    lsb_addr_mask = rnsam_ctx.non_hash_lsb_addr_mask;

    if (region_idx < RNSAM_HTG_REG_COUNT) {
        /* Configure the base address of the range */
        rnsam->SYS_CACHE_GRP_REGION[region_idx] = (base & ~lsb_addr_mask);
    } else {
        /* Configure the base address of the range */
        rnsam->HASHED_TGT_GRP_CFG1_REGION[region_idx - RNSAM_HTG_REG_COUNT] =
            (base & ~lsb_addr_mask);
    }

    /* Configure the end address of the region */
    rnsam->HASHED_TGT_GRP_CFG2_REGION[region_idx] =
        ((base + size - 1) & ~lsb_addr_mask);
}

/* Configurable base address & region size - Legacy CMN mode */
static int configure_htg_region_base_and_size(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size)
{
    uint8_t idx;
    uint64_t min_size;

    if ((base % size) != 0) {
        FWK_LOG_ERR(MOD_NAME "Invalid non-hashed region %u", region_idx);
        FWK_LOG_ERR(
            MOD_NAME "Base: 0x%llx should align with Size: 0x%llx", base, size);
        return FWK_E_PARAM;
    }

    min_size = rnsam_ctx.min_htg_size;

    if (region_idx < RNSAM_HTG_REG_COUNT) {
        idx = region_idx;
        /* Configure the memory region size */
        rnsam->SYS_CACHE_GRP_REGION[idx] =
            sam_encode_region_size(size, min_size)
            << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->SYS_CACHE_GRP_REGION[idx] |= (base / min_size)
            << RNSAM_REGION_ENTRY_BASE_POS;
    } else {
        idx = (region_idx - RNSAM_HTG_REG_COUNT);
        /* Configure the memory region size */
        rnsam->HASHED_TGT_GRP_CFG1_REGION[idx] =
            sam_encode_region_size(size, min_size)
            << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->HASHED_TGT_GRP_CFG1_REGION[idx] |= (base / min_size)
            << RNSAM_REGION_ENTRY_BASE_POS;
    }

    return FWK_SUCCESS;
}

/*
 * Get the non-hashed region end address programmed in the RNSAM register.
 */
static uint64_t get_non_hash_region_end_addr(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    uint64_t lsb_addr_mask;
    uint64_t end_addr;

    lsb_addr_mask = rnsam_ctx.non_hash_lsb_addr_mask;

    /* Read the end address of the programmed region */
    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        end_addr =
            (rnsam->NON_HASH_MEM_REGION_CFG2[region_idx] & ~lsb_addr_mask);
    } else {
        end_addr =
            (rnsam->NON_HASH_MEM_REGION_CFG2_GRP2
                 [region_idx - RNSAM_NON_HASH_REG_COUNT] &
             ~lsb_addr_mask);
    }

    return end_addr;
}

/*
 * Get the non-hashed region size programmed in the RNSAM register.
 */
static uint64_t get_non_hash_region_size(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    uint64_t encoded_size;

    /* Get the encoded memory region size */
    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        encoded_size =
            (rnsam->NON_HASH_MEM_REGION[region_idx] >>
             RNSAM_REGION_ENTRY_SIZE_POS);
    } else {
        encoded_size =
            (rnsam->NON_HASH_MEM_REGION_GRP2
                 [region_idx - RNSAM_NON_HASH_REG_COUNT] >>
             RNSAM_REGION_ENTRY_SIZE_POS);
    }

    encoded_size &= RNSAM_ENCODED_REGION_SIZE_MASK;
    return sam_decode_region_size(encoded_size, rnsam_ctx.min_non_hash_size);
}

void rnsam_stall(struct cmn_cyprus_rnsam_reg *rnsam)
{
    /* Stall RNSAM requests and enable default target ID selection */
    rnsam->STATUS = (rnsam->STATUS & ~RNSAM_STATUS_UNSTALL) |
        RNSAM_STATUS_USE_DEFAULT_TARGET_ID;
}

void rnsam_unstall(struct cmn_cyprus_rnsam_reg *rnsam)
{
    /* Unstall RNSAM requests and disable default target ID selection */
    rnsam->STATUS = (rnsam->STATUS | RNSAM_STATUS_UNSTALL) &
        ~(RNSAM_STATUS_USE_DEFAULT_TARGET_ID);
}

int rnsam_configure_non_hashed_region(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type target_type)
{
    int status;

    /*
     * If Range comparison mode is enabled, program the base and the
     * end address of the non-hashed region.
     */
    if (rnsam_ctx.non_hash_rcomp_en) {
        configure_non_hash_region_start_and_end_addr(
            rnsam, region_idx, base, size);
    } else {
        /* Program base address and size of the region */
        status = configure_non_hash_region_base_and_size(
            rnsam, region_idx, base, size);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* Configure the target node type for the non-hashed region */
    set_non_hash_region_target_type(rnsam, region_idx, target_type);

    return FWK_SUCCESS;
}

void rnsam_set_non_hashed_region_target(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint32_t region_idx,
    unsigned int node_id)
{
    unsigned int register_idx;
    uint8_t bit_pos;

    register_idx = region_idx / RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;

    bit_pos = RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
        (region_idx % RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);

    /* Clear the target node ID bitfield */
    rnsam->NON_HASH_TGT_NODEID[register_idx] &=
        ~(RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK << bit_pos);

    /* Set the target node ID */
    rnsam->NON_HASH_TGT_NODEID[register_idx] |=
        (node_id & RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK) << bit_pos;
}

void rnsam_set_non_hash_region_valid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    uint32_t register_idx;

    /* Mark the region valid for comparison */
    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        rnsam->NON_HASH_MEM_REGION[region_idx] |= RNSAM_REGION_ENTRY_VALID;
    } else {
        register_idx = (region_idx - RNSAM_NON_HASH_REG_COUNT);
        rnsam->NON_HASH_MEM_REGION_GRP2[register_idx] |=
            RNSAM_REGION_ENTRY_VALID;
    }
}

int rnsam_configure_hashed_region(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type target_type)
{
    int status;

    /*
     * If Range comparison mode is enabled, program the base and the
     * end address of the non-hashed region.
     */
    if (rnsam_ctx.htg_rcomp_en) {
        configure_htg_region_start_and_end_addr(rnsam, region_idx, base, size);
    } else {
        status =
            configure_htg_region_base_and_size(rnsam, region_idx, base, size);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* Configure the target node type for the hashed region */
    set_htg_region_target_type(rnsam, region_idx, target_type);

    return FWK_SUCCESS;
}

void rnsam_set_htg_target_hn_nodeid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint16_t hn_node_id,
    uint32_t hn_node_id_idx)
{
    unsigned int register_idx;
    uint8_t bit_pos;

    register_idx = hn_node_id_idx / RNSAM_SCG_TARGET_ENTRIES_PER_GROUP;

    bit_pos = RNSAM_SCG_TARGET_ENTRY_BITS_WIDTH *
        ((hn_node_id_idx % (RNSAM_SCG_TARGET_ENTRIES_PER_GROUP)));

    /* Configure target HN-F node ID */
    rnsam->SYS_CACHE_GRP_HN_NODEID[register_idx] += (uint64_t)hn_node_id
        << bit_pos;
}

void rnsam_set_htg_target_hn_count(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint16_t target_count)
{
    /* Configure the number of target nodes in this SCG */
    rnsam->SYS_CACHE_GROUP_HN_COUNT |= ((uint64_t)target_count)
        << (RNSAM_SCG_HN_CNT_POS(scg_idx));
}

void rnsam_enable_htg_cal_mode(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx)
{
    /* Enable the CAL mode for the given SCG region */
    rnsam->SYS_CACHE_GRP_CAL_MODE |=
        (RNSAM_SCG_HNS_CAL_MODE_EN << (scg_idx * RNSAM_SCG_HNS_CAL_MODE_SHIFT));
}

void rnsam_set_htg_region_valid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    if (region_idx < RNSAM_HTG_REG_COUNT) {
        /* Configure the bits [51:16] of base address of the range */
        rnsam->SYS_CACHE_GRP_REGION[region_idx] |= RNSAM_REGION_ENTRY_VALID;

    } else {
        /* Configure the bits [51:16] of base address of the range */
        rnsam->HASHED_TGT_GRP_CFG1_REGION[region_idx - RNSAM_HTG_REG_COUNT] |=
            RNSAM_REGION_ENTRY_VALID;
    }
}

uint64_t rnsam_get_non_hashed_region_base(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    uint64_t lsb_addr_mask;
    uint32_t register_idx;

    lsb_addr_mask = rnsam_ctx.non_hash_lsb_addr_mask;

    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        /* Return the base address programmed in the register */
        return (rnsam->NON_HASH_MEM_REGION[region_idx] & ~lsb_addr_mask);
    } else {
        register_idx = (region_idx - RNSAM_NON_HASH_REG_COUNT);
        /* Return the base address programmed in the register */
        return (rnsam->NON_HASH_MEM_REGION_GRP2[register_idx] & ~lsb_addr_mask);
    }
}

uint64_t rnsam_get_non_hashed_region_size(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    uint64_t base;
    uint64_t size;
    uint64_t end_addr;

    /*
     * If Range comparison mode is enabled, get the end address and then
     * calculate the region size otherwise get the size of the non-hashed
     * region.
     */
    if (rnsam_ctx.non_hash_rcomp_en) {
        end_addr = get_non_hash_region_end_addr(rnsam, region_idx);
        base = rnsam_get_non_hashed_region_base(rnsam, region_idx);

        fwk_assert(base > 0);

        size = (end_addr - base);
    } else {
        /* Program base address and size of the region */
        size = get_non_hash_region_size(rnsam, region_idx);
    }

    return size;
}

unsigned int rnsam_get_non_hashed_region_target_id(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx)
{
    uint32_t register_idx;
    uint32_t bit_pos;

    register_idx = region_idx / RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP;
    bit_pos = RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH *
        (region_idx % RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP);

    /* Return the target node ID programmed in the register */
    return (rnsam->NON_HASH_TGT_NODEID[register_idx] >> bit_pos) &
        RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK;
}

void rnsam_set_hier_hash_cluster_groups(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_cluster_groups)
{
    /* Configure the number of clusters */
    rnsam->HASHED_TARGET_GRP_HASH_CNTL[scg_idx] |=
        (num_cluster_groups << RNSAM_HIER_HASH_CLUSTERS_POS);
}

void rnsam_set_hier_hash_num_hns_per_cluster(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_hns_per_cluster)
{
    /* Configure the number of HN-S nodes in each cluster */
    rnsam->HASHED_TARGET_GRP_HASH_CNTL[scg_idx] |=
        (num_hns_per_cluster << RNSAM_HIER_HASH_NODES_POS);
}

void rnsam_set_hier_hash_addr_striping(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_cluster_groups)
{
    /*
     * Configure number of address bits needs to shuttered (removed) at
     * second hierarchy hash.
     *
     * 3'b000: no address shuttering
     * 3'b001: one addr bit shuttered (2 clusters)
     * 3'b010: two addr bit shuttered (4 clusters)
     * 3'b011: three addr bit shuttered (8 clusters)
     * 3'b100: four addr bit shuttered (16 clusters)
     * 3'b101: five addr bit shuttered (32 clusters)
     * others: Reserved
     */
    rnsam->HASHED_TARGET_GRP_HASH_CNTL[scg_idx] |=
        (fwk_math_log2((unsigned int)num_cluster_groups)
         << RNSAM_HIER_ENABLE_ADDRESS_STRIPING_POS);
}

void rnsam_enable_hier_hash_mode(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx)
{
    /* Enable hierarchical hashing mode */
    rnsam->HASHED_TARGET_GRP_HASH_CNTL[scg_idx] |=
        (RNSAM_HIERARCHICAL_HASH_EN_MASK << RNSAM_HIERARCHICAL_HASH_EN_POS);
}

/* Enable CPA mode for the given non-hashed region */
int rnsam_enable_non_hash_region_cpa_mode(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t region_idx)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    register_idx = (region_idx / CPA_MODE_CTRL_REGIONS_PER_GROUP);
    if (register_idx >= RNSAM_CPA_MODE_CTRL_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos =
        ((region_idx % CPA_MODE_CTRL_REGIONS_PER_GROUP) *
         CPA_MODE_CTRL_PAG_WIDTH_PER_REGION);

    rnsam->CML_PORT_AGGR_MODE_CTRL_REG[register_idx] |=
        (UINT64_C(0x1) << bit_pos);

    return FWK_SUCCESS;
}

/*
 * Configure CPA Group ID for the given non-hashed region. The outbound requests
 * are distributed across this CPA Group.
 */
int rnsam_configure_non_hash_region_cpag_id(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t region_idx,
    uint8_t cpag_id)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    register_idx = (region_idx / CPA_MODE_CTRL_REGIONS_PER_GROUP);
    if (register_idx >= RNSAM_CPA_MODE_CTRL_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos =
        (((region_idx % CPA_MODE_CTRL_REGIONS_PER_GROUP) *
          CPA_MODE_CTRL_PAG_WIDTH_PER_REGION) +
         CPA_MODE_CTRL_PAG_GRPID_OFFSET);

    rnsam->CML_PORT_AGGR_MODE_CTRL_REG[register_idx] |=
        ((uint64_t)(cpag_id & CPA_MODE_CTRL_PAG_GRPID_MASK) << bit_pos);

    return FWK_SUCCESS;
}

/* Configure the number of CCG nodes in the CPA Group */
int rnsam_configure_cpag_ccg_count(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_id,
    uint8_t ccg_count)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    if (!is_cpag_ccg_count_valid(ccg_count)) {
        return FWK_E_PARAM;
    }

    register_idx = (cpag_id / CPA_CTRL_CPAG_PER_GROUP);
    if (register_idx >= RNSAM_CPA_CTRL_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos =
        ((cpag_id % CPA_CTRL_CPAG_PER_GROUP) * CPA_CTRL_NUM_CXG_PAG_WIDTH);

    rnsam->CML_PORT_AGGR_CTRL_REG[register_idx] |=
        ((get_num_cxg_value(ccg_count) & CPA_CTRL_NUM_CXG_PAG_MASK) << bit_pos);

    return FWK_SUCCESS;
}

/* Configure the CPA Group port type as SMP or CXL */
int rnsam_set_cpag_port_type(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_id,
    enum cpa_port_type port_type)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    register_idx = (cpag_id / CPA_CTRL_CPAG_PER_GROUP);
    if (register_idx >= RNSAM_CPA_CTRL_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos =
        (((cpag_id % CPA_CTRL_CPAG_PER_GROUP) * CPA_CTRL_NUM_CXG_PAG_WIDTH) +
         CPA_CTRL_PORT_TYPE_POS);

    rnsam->CML_PORT_AGGR_CTRL_REG[register_idx] |= (port_type << bit_pos);

    return FWK_SUCCESS;
}

/* Configure the CHI Node ID of the target node in the CPA Group */
int rnsam_configure_cpag_target_id(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_tgt_idx,
    unsigned int ccg_ra_node_id)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    register_idx = (cpag_tgt_idx / CPAG_TGTID_PER_GROUP);
    if (register_idx >= RNSAM_CPA_GRP_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos = ((cpag_tgt_idx * CPAG_TGTID_WIDTH) % CPAG_TGTID_WIDTH_PER_GROUP);

    rnsam->CML_PORT_AGGR_GRP_REG[register_idx] |=
        (ccg_ra_node_id & CPAG_TGTID_MASK) << bit_pos;

    return FWK_SUCCESS;
}

/*
 * Configure the base index for the CPA Group.
 *
 * Each CPAG derives the PAG target ID based on the address hashing and the hash
 * index is looked up into the CML target ID table. Base index for each CPAG is
 * derived based on the linked list mechanism.
 */
int rnsam_configure_cpag_base_index(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_id,
    uint8_t cpag_base_idx)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    register_idx = (cpag_id / CPAG_BASE_INDX_CPAG_PER_GROUP);
    if (register_idx >= RNSAM_CPAG_BASE_INDX_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos =
        ((cpag_id % CPAG_BASE_INDX_CPAG_PER_GROUP) *
         CPAG_BASE_INDX_WIDTH_PER_CPAG);

    rnsam->CML_CPAG_BASE_INDX_GRP[register_idx] &=
        ~(CPAG_BASE_INDX_MASK << bit_pos);

    rnsam->CML_CPAG_BASE_INDX_GRP[register_idx] |= (cpag_base_idx << bit_pos);

    return FWK_SUCCESS;
}

int setup_rnsam_ctx(struct cmn_cyprus_rnsam_reg *rnsam)
{
    /* Return if initialized already */
    if (rnsam_ctx.is_initialized) {
        return FWK_SUCCESS;
    }

    if (rnsam == NULL) {
        return FWK_E_PARAM;
    }

    /*
     * Check if the start and end address has to be programmed for
     * non-hashed regions.
     */
    rnsam_ctx.non_hash_rcomp_en =
        is_non_hash_range_comparison_mode_enabled(rnsam);

    /*
     * Check if the start and end address has to be programmed for
     * hashed target group regions.
     */
    rnsam_ctx.htg_rcomp_en = is_htg_range_comparison_mode_enabled(rnsam);

    /* Get the LSB mask from LSB bit position defining minimum region size */
    rnsam_ctx.non_hash_lsb_addr_mask = get_rnsam_non_hash_lsb_addr_mask(rnsam);

    /* Get the LSB mask from LSB bit position defining minimum region size */
    rnsam_ctx.htg_lsb_addr_mask = get_rnsam_htg_lsb_addr_mask(rnsam);

    /* Get the minimum region size */
    rnsam_ctx.min_non_hash_size = get_min_non_hash_region_size(rnsam);
    rnsam_ctx.min_htg_size = get_min_htg_size(rnsam);

    rnsam_ctx.is_initialized = true;

    return FWK_SUCCESS;
}
