/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SMCF_UTIL_H
#define __SMCF_UTIL_H

#include <stdint.h>

uint32_t get_next_smcf_ram_offset(
    uint32_t this_offset,
    uint32_t header_format,
    uint32_t data_size,
    uint32_t tag_len_in_bits);

#endif
