/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Common definitions and utility functions for the CMN-Cyprus module.
 */

#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_ctx.h>

#include <fwk_assert.h>
#include <fwk_math.h>

#include <stdint.h>

uint64_t sam_encode_region_size(uint64_t region_size, uint64_t min_size)
{
    uint64_t blocks;

    /* Region size must be a multiple of minimum region size */
    fwk_assert((region_size % min_size) == 0);

    /* Region size also must be a power of two */
    fwk_assert((region_size & (region_size - 1)) == 0);

    blocks = region_size / min_size;
    return fwk_math_log2(blocks);
}

uint64_t sam_decode_region_size(uint64_t encoded_size, uint64_t min_size)
{
    uint64_t blocks;
    uint64_t size;

    /*
     * Calculate the number of blocks.
     * Encoded region size is binary logarithm of number of blocks.
     */
    blocks = fwk_math_pow2(encoded_size);

    /*
     * Calculate the size of the region.
     * Number of blocks = (size / min_size)
     */
    size = blocks * min_size;

    return size;
}

inline bool is_exp_raid_valid(uint16_t raid)
{
    return (raid <= MAX_EXP_RAID);
}

inline bool is_ccg_agent_id_valid(unsigned int agent_id)
{
    return (agent_id <= MAX_LOGICAL_AGENT_ID);
}

inline bool is_ccg_link_id_valid(uint8_t link_id)
{
    return (link_id == CCG_LINK_0);
}

/* Helper function to get the number of HN-S nodes in the given SCG */
unsigned int get_scg_hns_count(
    const struct cmn_cyprus_ctx *ctx,
    uint8_t scg_idx)
{
    unsigned int hns_count;

    fwk_assert(ctx != NULL);
    fwk_assert(scg_idx < ctx->scg_count);

    hns_count = ctx->scg_hns_count[scg_idx];

    /*
     * When CAL mode is enabled, the node ID of only one of the two nodes
     * connected to the CAL, is entered into the applicable Target ID table.
     * So, the adjusted HN-S count is half of the HN-S nodes in the SCG.
     */
    return (ctx->config->hns_cal_mode == true) ? (hns_count / 2) : hns_count;
}
