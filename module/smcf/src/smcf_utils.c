/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "smcf_data.h"
#include "smcf_utils.h"

#include <stddef.h>

#define SMCF_UTILS_ALIGNMENT UINT32_C(4)
#define WORD_SIZE            UINT32_C(4)
#define DOUBLE_WORD_SIZE     UINT32_C(8)

#define NUM_OF_BYTES(x) (x / 8)

static uint32_t memory_align(uint32_t address, size_t alignment)
{
    uint32_t compensation = 0;

    if ((address % alignment) != 0) {
        compensation = alignment - (address % alignment);
    }

    address = address + compensation;

    return address;
}

uint32_t get_next_smcf_ram_offset(
    uint32_t this_offset,
    uint32_t header_format,
    uint32_t data_size,
    uint32_t tag_len_in_bits)
{
    uint32_t header_size = 0;
    uint32_t next_offset = 0;

    if ((header_format & SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID) != 0) {
        header_size += WORD_SIZE;
    }

    if ((header_format & SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS) != 0) {
        header_size += WORD_SIZE;
    }

    if ((header_format & SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID) != 0) {
        header_size += WORD_SIZE;

        if ((header_format & SMCF_SAMPLE_HEADER_FORMAT_END_ID) != 0) {
            header_size += WORD_SIZE;
        }
    }

    if ((header_format & SMCF_SAMPLE_HEADER_FORMAT_TAG_ID) != 0) {
        uint32_t tag_lenght_in_bytes = NUM_OF_BYTES(tag_len_in_bits);
        header_size += WORD_SIZE + tag_lenght_in_bytes;

        if ((header_format & SMCF_SAMPLE_HEADER_FORMAT_END_ID) != 0) {
            header_size += WORD_SIZE + tag_lenght_in_bytes;
        }
    }

    next_offset = this_offset + header_size + data_size;
    next_offset = memory_align(next_offset, SMCF_UTILS_ALIGNMENT);

    return next_offset;
}
