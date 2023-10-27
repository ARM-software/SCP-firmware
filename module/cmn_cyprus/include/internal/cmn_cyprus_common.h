/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Common definitions and utility functions for the CMN-Cyprus module.
 */

#ifndef CMN_CYPRUS_COMMON_INTERNAL_H
#define CMN_CYPRUS_COMMON_INTERNAL_H

#include <stdint.h>

/* SAM Granularity of RN-SAM and HN-F SAM */
#define SAM_GRANULARITY (64 * FWK_MIB)

/*
 * Convert a memory region size into a size format used by the CMN-CYPRUS
 * registers. The format is the binary logarithm of the memory region size
 * represented as blocks multiple of the CMN-CYPRUS's granularity:
 * n =  log2(size / SAM_GRANULARITY)
 *
 * \note Only applicable when RCOMP mode is not enabled.
 *
 * \param size Memory region size to be converted
 *      \pre size must be a multiple of SAM_GRANULARITY
 *
 * \return log2(size / SAM_GRANULARITY)
 */
uint64_t sam_encode_region_size(uint64_t size);

/*
 * Convert encoded memory region size to the region size.
 *
 * \note Only applicable when RCOMP mode is not enabled.
 *
 * \param encoded_size Encoded Memory region size to be converted
 *
 * \return size
 */
uint64_t sam_decode_region_size(uint64_t encoded_size);

#endif /* CMN_CYPRUS_COMMON_INTERNAL_H */
