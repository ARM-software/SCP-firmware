/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Common definitions and utility functions for the CMN-Cyprus module.
 */

#ifndef CMN_CYPRUS_COMMON_INTERNAL_H
#define CMN_CYPRUS_COMMON_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

/* Maximum Expanded RAID value */
#define MAX_EXP_RAID 1023

/* Maximum Logical Agent ID */
#define MAX_LOGICAL_AGENT_ID 63

/*
 * CCG supports only one link and all valid agents must be programmed with
 * LinkID 0.
 */
#define CCG_LINK_0 0

#define GET_CHIP_ADDR_OFFSET(ctx_ptr) \
    (ctx_ptr->config->chip_addr_space * ctx_ptr->chip_id)

/*
 * Convert a memory region size into a size format used by the CMN-CYPRUS
 * registers. The format is the binary logarithm of the memory region size
 * represented as blocks multiple of the CMN-CYPRUS's granularity:
 * n =  log2 (region_size / min_size)
 *
 * \note Only applicable when RCOMP mode is not enabled.
 *
 * \param region_size Memory region size to be converted
 *      \pre size must be a multiple of min_size
 * \param min_size Minimum size (granularity) of the regions
 *      \pre Minimum size must be valid.
 *
 * \return log2 (region_size / min_size)
 */
uint64_t sam_encode_region_size(uint64_t region_size, uint64_t min_size);

/*
 * Convert encoded memory region size to the region size.
 *
 * \note Only applicable when RCOMP mode is not enabled.
 *
 * \param encoded_size Encoded Memory region size to be converted
 * \param min_size Minimum region size(granularity) value that was used to
 *      encode the region.
 *      \pre Minimum size must be valid.
 *
 * \return region size
 */
uint64_t sam_decode_region_size(uint64_t encoded_size, uint64_t min_size);

/*
 * Check if the given expanded raid is valid.
 *
 * \param raid Expanded RAID.
 *
 * \return true if (raid <= MAX_EXP_RAID)
 */
bool is_exp_raid_valid(uint16_t raid);

/*
 * Check if the given agent id is valid.
 *
 * \param agent_id Agent logical id.
 *
 * \return true if (agent_id <= MAX_LOGICAL_AGENT_ID)
 */
bool is_ccg_agent_id_valid(unsigned int agent_id);

/*
 * Check if the given link id is valid.
 *
 * \param link_id CCG link id.
 *
 * \return true if (link_id == CCG_LINK_0)
 */
bool is_ccg_link_id_valid(uint8_t link_id);

#endif /* CMN_CYPRUS_COMMON_INTERNAL_H */
