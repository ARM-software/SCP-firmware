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

#include <stdint.h>

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

#endif /* CMN_CYPRUS_COMMON_INTERNAL_H */
