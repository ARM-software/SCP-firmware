/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCF_DATA_H
#define SMCF_DATA_H

#include "mgi.h"

#include <mod_smcf_data.h>

#include <fwk_macros.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

#define DATA_BITS_MASK(data_width) ((1 << data_width) - 1)

/* data width in bit */
enum data_width {
    DATA_WIDTH_32_BITS = 32,
    DATA_WIDTH_16_BITS = 16,
    DATA_WIDTH_8_BITS = 8,
};

/* Packed Data alignment */
enum packed_data_alignment {
    PACKED_DATA_ALIGN_8_BITS = 8,
    PACKED_DATA_ALIGN_16_BITS = 16,
    PACKED_DATA_ALIGN_32_BITS = 32,
};

struct data_header {
    uint32_t format;
    uint32_t FWK_R *group_id_addr;
    uint32_t FWK_R *valid_bits_addr;
    uint32_t FWK_R *start_count_id_addr;
    uint32_t FWK_R *end_count_id_addr;
    uint32_t FWK_R *start_tag_length_addr;
    uint32_t FWK_R *start_tag_addr;
    uint32_t FWK_R *end_tag_length_addr;
    uint32_t FWK_R *end_tag_addr;
};

struct smcf_data_attr {
    /* Data header */
    struct data_header header;

    /* Data address in the MGI or other memory space */
    uint32_t FWK_R *data_addr;

    /* Number of data samples from each MLI */
    uint32_t num_of_data;

    /* Each data sample width */
    uint32_t data_width;

    /* Data packed or not for data <= 16-bits */
    bool packed;
};

int smcf_data_get_data(
    const struct smcf_data_attr data_attr,
    const uint32_t monitor_index,
    uint32_t *const data_dest_addr,
    uint32_t *const tag_dest_addr);

uint32_t smcf_data_get_data_buffer_size(const struct smcf_data_attr data_attr);

int smcf_data_set_data_address(
    struct smcf_mgi_reg *mgi,
    struct mod_smcf_data_config data_config,
    struct smcf_data_attr *data_attr);

uint32_t smcf_data_get_tag_length(const struct data_header header);

uint32_t smcf_data_get_group_id(const struct smcf_data_attr data_attributes);

#endif /* SMCF_DATA_H */
