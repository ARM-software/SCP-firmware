/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
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

    /*! Hashed regions range comparison mode status flag */
    bool htg_rcomp_en;

    /*!
     * LSB address mask for programming hashed target group region base address
     * and end address.
     */
    uint64_t htg_lsb_addr_mask;

    /*! Flag to indicate that the RNSAM context has been initialized */
    bool is_initialized;
};

static struct cmn_cyprus_rnsam_ctx rnsam_ctx;

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
        /* Configure the bits [51:16] of base address of the range */
        rnsam->SYS_CACHE_GRP_REGION[region_idx] |=
            (target_type << RNSAM_REGION_ENTRY_TARGET_TYPE_POS);

    } else {
        /* Configure the bits [51:16] of base address of the range */
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
        /* Configure the bits [51:16] of base address of the range */
        rnsam->NON_HASH_MEM_REGION[region_idx] = (base & ~lsb_addr_mask);

        /* Configure the end address of the region */
        rnsam->NON_HASH_MEM_REGION_CFG2[region_idx] =
            ((base + size - 1) & ~lsb_addr_mask);
    } else {
        /* Configure the bits [51:16] of base address of the range */
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
    if ((base % size) != 0) {
        FWK_LOG_ERR(MOD_NAME "Invalid non-hashed region %u", region_idx);
        FWK_LOG_ERR(
            MOD_NAME "Base: 0x%llx should align with Size: 0x%llx", base, size);
        return FWK_E_PARAM;
    }

    if (region_idx < RNSAM_NON_HASH_REG_COUNT) {
        /* Configure the memory region size */
        rnsam->NON_HASH_MEM_REGION[region_idx] = sam_encode_region_size(size)
            << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->NON_HASH_MEM_REGION[region_idx] |= (base / SAM_GRANULARITY)
            << RNSAM_REGION_ENTRY_BASE_POS;

    } else {
        /* Configure the memory region size */
        rnsam->NON_HASH_MEM_REGION_GRP2[region_idx - RNSAM_NON_HASH_REG_COUNT] =
            sam_encode_region_size(size) << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam
            ->NON_HASH_MEM_REGION_GRP2[region_idx - RNSAM_NON_HASH_REG_COUNT] |=
            (base / SAM_GRANULARITY) << RNSAM_REGION_ENTRY_BASE_POS;
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
    if ((base % size) != 0) {
        FWK_LOG_ERR(MOD_NAME "Invalid non-hashed region %u", region_idx);
        FWK_LOG_ERR(
            MOD_NAME "Base: 0x%llx should align with Size: 0x%llx", base, size);
        return FWK_E_PARAM;
    }

    if (region_idx < RNSAM_HTG_REG_COUNT) {
        /* Configure the memory region size */
        rnsam->SYS_CACHE_GRP_REGION[region_idx] = sam_encode_region_size(size)
            << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->SYS_CACHE_GRP_REGION[region_idx] = (base / SAM_GRANULARITY)
            << RNSAM_REGION_ENTRY_BASE_POS;
    } else {
        /* Configure the memory region size */
        rnsam->HASHED_TGT_GRP_CFG1_REGION[region_idx - RNSAM_HTG_REG_COUNT] =
            sam_encode_region_size(size) << RNSAM_REGION_ENTRY_SIZE_POS;

        /* Configure the base address of the range */
        rnsam->HASHED_TGT_GRP_CFG1_REGION[region_idx - RNSAM_HTG_REG_COUNT] =
            (base / SAM_GRANULARITY) << RNSAM_REGION_ENTRY_BASE_POS;
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
    return sam_decode_region_size(encoded_size);
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

    rnsam_ctx.is_initialized = true;

    return FWK_SUCCESS;
}
