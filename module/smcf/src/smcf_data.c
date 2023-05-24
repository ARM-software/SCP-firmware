/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mod_smcf_data.h"
#include "smcf_data.h"

#include <fwk_assert.h>

#include <stddef.h>

static void smcf_memcpy_32bit(
    uint32_t *dest,
    volatile const uint32_t *src,
    size_t count)
{
    uint32_t copy_index;

    for (copy_index = 0; copy_index < count; copy_index++) {
        dest[copy_index] = src[copy_index];
    }
}

static void smcf_copy_unpacked_data(
    const struct smcf_data_attr data_attributes,
    const unsigned int monitor_index,
    uint32_t *const dest_addr)
{
    uint32_t volatile const *src_addr = data_attributes.data_addr;
    uint32_t num_of_data = data_attributes.num_of_data;
    uint32_t data_width = data_attributes.data_width;
    uint32_t volatile const *data_address;

    if (data_width > DATA_WIDTH_32_BITS) {
        /* Data width = 33 to 64 bits */
        data_address = src_addr + (monitor_index * num_of_data * 2);
        smcf_memcpy_32bit(dest_addr, data_address, 2 * num_of_data);

    } else {
        /* Data width = 32 bits */
        data_address = src_addr + (monitor_index * num_of_data);
        smcf_memcpy_32bit(dest_addr, data_address, num_of_data);
    }
}

static void smcf_memcpy_packed(
    uint32_t *dest,
    volatile const uint32_t *src,
    const unsigned int monitor_index,
    size_t count,
    uint32_t data_width)
{
    uint32_t mask = DATA_BITS_MASK(data_width);
    uint32_t sample_shift;
    uint32_t copy_index;
    uint32_t word_idx;
    uint32_t factor;
    uint32_t align;

    align =
        ((data_width > DATA_WIDTH_8_BITS) ? PACKED_DATA_ALIGN_16_BITS :
                                            PACKED_DATA_ALIGN_8_BITS);

    for (copy_index = 0; copy_index < count; copy_index++) {
        factor = (align * (copy_index + (monitor_index * count)));
        sample_shift = factor % DATA_WIDTH_32_BITS;
        word_idx = factor / DATA_WIDTH_32_BITS;
        dest[copy_index] = (src[word_idx] >> sample_shift) & mask;
    }
}

static void smcf_copy_packed_data(
    const struct smcf_data_attr data_attributes,
    const unsigned int monitor_index,
    uint32_t *const dest_addr)
{
    uint32_t volatile const *src_addr = data_attributes.data_addr;
    uint32_t num_of_data = data_attributes.num_of_data;
    uint32_t data_width = data_attributes.data_width;

    smcf_memcpy_packed(
        dest_addr, src_addr, monitor_index, num_of_data, data_width);
}

void smcf_copy_data(
    const struct smcf_data_attr data_attributes,
    const unsigned int monitor_index,
    uint32_t *const dest_addr)
{
    uint32_t data_width = data_attributes.data_width;
    bool packed = data_attributes.packed;

    if (packed && (data_width <= DATA_WIDTH_16_BITS)) {
        smcf_copy_packed_data(data_attributes, monitor_index, dest_addr);
    } else {
        smcf_copy_unpacked_data(data_attributes, monitor_index, dest_addr);
    }
}

uint32_t smcf_data_get_data_buffer_size(
    const struct smcf_data_attr data_attributes)
{
    uint32_t num_of_data = data_attributes.num_of_data;
    uint32_t data_width = data_attributes.data_width;

    return (data_width > DATA_WIDTH_32_BITS) ? (2 * num_of_data) : num_of_data;
}

static int smcf_set_data_address_mgi(
    struct smcf_mgi_reg *const mgi,
    struct smcf_data_attr *const data_attributes)
{
    if ((mgi == 0) || (data_attributes == 0)) {
        return FWK_E_PARAM;
    }

    data_attributes->data_addr = mgi->DATA;
    data_attributes->header.group_id_addr = &mgi->GRP_ID;
    data_attributes->header.valid_bits_addr = &mgi->DVLD;
    data_attributes->header.start_count_id_addr = &mgi->SMPID_START;
    data_attributes->header.end_count_id_addr = &mgi->SMPID_END;
    data_attributes->header.start_tag_addr = &mgi->TAG0;
    data_attributes->header.end_tag_addr = &mgi->TAG0;

    return FWK_SUCCESS;
}

static inline bool is_header_include_group_id(const uint32_t header_format)
{
    return (header_format & SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID) != 0;
}

static inline bool is_header_include_valid_bits(const uint32_t header_format)
{
    return (header_format & SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS) != 0;
}

static inline bool is_header_include_count_id(const uint32_t header_format)
{
    return (header_format & SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID) != 0;
}

static inline bool is_header_include_tag_id(const uint32_t header_format)
{
    return (header_format & SMCF_SAMPLE_HEADER_FORMAT_TAG_ID) != 0;
}

static inline bool is_header_include_end_id(const uint32_t header_format)
{
    return (header_format & SMCF_SAMPLE_HEADER_FORMAT_END_ID) != 0;
}

static inline unsigned int smcf_data_header_add_group_id(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_group_id(header_format)) {
        mgi_enable_group_id_write_to_ram(mgi);
        addr_offset++;
    }

    return addr_offset;
}

static inline unsigned int smcf_data_header_add_valid_bits(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_valid_bits(header_format)) {
        mgi_enable_valid_bits_write_to_ram(mgi);
        addr_offset++;
    }

    return addr_offset;
}

static inline unsigned int smcf_data_header_add_start_count_id(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_count_id(header_format)) {
        mgi_enable_count_id_write_to_ram(mgi);
        mgi_request_start_id_wirte_to_ram(mgi);
        addr_offset++;
    }

    return addr_offset;
}

static inline unsigned int smcf_data_header_add_end_count_id(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_count_id(header_format) &&
        is_header_include_end_id(header_format)) {
        mgi_request_start_and_end_id_wirte_to_ram(mgi);
        addr_offset++;
    }

    return addr_offset;
}

static inline uint32_t get_number_of_32_bits(const uint32_t number_of_bits)
{
    return (number_of_bits + 31) / 32;
}

static inline unsigned int smcf_data_header_add_tag_length(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_tag_id(header_format)) {
        addr_offset++;
    }

    return addr_offset;
}

static inline unsigned int smcf_data_header_add_start_tag(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_tag_id(header_format)) {
        mgi_enable_tag_id_write_to_ram(mgi);
        mgi_request_start_id_wirte_to_ram(mgi);
        addr_offset = get_number_of_32_bits(mgi_get_tag_length_in_bits(mgi));
    }

    return addr_offset;
}

static inline unsigned int smcf_data_header_add_end_tag(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    unsigned int addr_offset = 0;

    if (is_header_include_tag_id(header_format) &&
        is_header_include_end_id(header_format)) {
        mgi_request_start_and_end_id_wirte_to_ram(mgi);
        addr_offset = get_number_of_32_bits(mgi_get_tag_length_in_bits(mgi));
    }

    return addr_offset;
}

static unsigned int smcf_data_include_header(
    struct smcf_mgi_reg *const mgi,
    const struct mod_smcf_data_config data_config,
    struct smcf_data_attr *const data_attributes)
{
    const uint32_t *read_addr = data_config.read_addr;
    uint32_t header_format = data_config.header_format;
    unsigned int addr_offset = 0;

    data_attributes->header.format = header_format;

    data_attributes->header.group_id_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_group_id(mgi, header_format);

    data_attributes->header.valid_bits_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_valid_bits(mgi, header_format);

    data_attributes->header.start_count_id_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_start_count_id(mgi, header_format);

    data_attributes->header.end_count_id_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_end_count_id(mgi, header_format);

    data_attributes->header.start_tag_length_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_tag_length(mgi, header_format);

    data_attributes->header.start_tag_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_start_tag(mgi, header_format);

    data_attributes->header.end_tag_length_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_tag_length(mgi, header_format);

    data_attributes->header.end_tag_addr = read_addr + addr_offset;
    addr_offset += smcf_data_header_add_end_tag(mgi, header_format);

    return addr_offset;
}

static int smcf_set_data_address_ram(
    struct smcf_mgi_reg *const mgi,
    struct mod_smcf_data_config data_config,
    struct smcf_data_attr *const data_attributes)
{
    uint32_t FWK_R *address;
    int status;

    if ((data_config.read_addr == 0) || (data_attributes == 0)) {
        return FWK_E_PARAM;
    }

    if (!mgi_is_dma_supported(mgi)) {
        return FWK_E_SUPPORT;
    }

    address = data_config.read_addr;

    address += smcf_data_include_header(mgi, data_config, data_attributes);

    data_attributes->data_addr = address;

    status = mgi_set_dma_data_address(mgi, data_config.write_addr);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return mgi_enable_dma(mgi);
}

static int smcf_validate_header_format(
    struct smcf_mgi_reg *const mgi,
    const uint32_t header_format)
{
    if (is_header_include_tag_id(header_format)) {
        if (!mgi_is_tag_supported(mgi)) {
            return FWK_E_SUPPORT;
        }
    }

    if (is_header_include_end_id(header_format)) {
        if (!(is_header_include_count_id(header_format) ||
              is_header_include_tag_id(header_format))) {
            return FWK_E_PARAM;
        }
    }

    return FWK_SUCCESS;
}

int smcf_data_set_data_address(
    struct smcf_mgi_reg *const mgi,
    const struct mod_smcf_data_config data_config,
    struct smcf_data_attr *const data_attributes)
{
    int status;

    data_attributes->header.format = data_config.header_format;

    status = smcf_validate_header_format(mgi, data_attributes->header.format);
    if (status != FWK_SUCCESS) {
        return status;
    }

    switch (data_config.data_location) {
    case SMCF_DATA_LOCATION_MGI:
        status = smcf_set_data_address_mgi(mgi, data_attributes);
        break;

    case SMCF_DATA_LOCATION_RAM:
        status = smcf_set_data_address_ram(mgi, data_config, data_attributes);
        break;
    default:
        /* Invalid config */
        fwk_unexpected();
        status = FWK_E_SUPPORT;
    }

    return status;
}

static bool is_sample_id_status_count_id(const uint32_t sample_id_encoded)
{
    return ((sample_id_encoded & SMCF_MGI_SMPID_STATUS) >>
            SMCF_MGI_SMPID_STATUS_POS) == SMCF_SAMPLE_ID_VALID_COUNT;
}

static bool is_sample_id_status_tag_length(const uint32_t sample_id_encoded)
{
    return ((sample_id_encoded & SMCF_MGI_SMPID_STATUS) >>
            SMCF_MGI_SMPID_STATUS_POS) == SMCF_SAMPLE_ID_VALID_TAG_LEN;
}

static bool check_valid_bits(
    const struct data_header header,
    const unsigned int mli_index)
{
    return (is_header_include_valid_bits(header.format)) ?
        (((*header.valid_bits_addr) >> mli_index) == 1) :
        true;
}

static bool check_count_id(const struct data_header header)
{
    return (is_header_include_count_id(header.format)) ?
        is_sample_id_status_count_id(*header.start_count_id_addr) :
        true;
}

static bool check_tag_id(const struct data_header header)
{
    return (is_header_include_tag_id(header.format)) ?
        is_sample_id_status_tag_length(*header.start_tag_length_addr) :
        true;
}

static bool is_tag_state_valid(const struct data_header header)
{
    return (is_header_include_tag_id(header.format)) ?
        is_sample_id_status_tag_length(*header.start_tag_length_addr) :
        false;
}

static bool check_end_id(const struct data_header header)
{
    return (is_header_include_end_id(header.format) &&
            is_header_include_count_id(header.format)) ?
        (*header.start_count_id_addr == *header.end_count_id_addr) :
        true;
}

static bool smcf_data_is_sample_valid_before_copy(
    const struct data_header header,
    const unsigned int mli_index)
{
    return check_valid_bits(header, mli_index) && check_count_id(header) &&
        check_tag_id(header) && check_end_id(header);
}

static uint32_t get_start_sample_id_value(const struct data_header header)
{
    if (!is_header_include_count_id(header.format)) {
        return 0;
    }

    if (!is_sample_id_status_count_id(*header.start_count_id_addr)) {
        return 0;
    }

    return *header.start_count_id_addr & SMCF_MGI_SMPID_VALUE;
}

static bool is_count_id_match(
    const uint32_t sample_id_encoded,
    const uint32_t current_count)
{
    return (sample_id_encoded & SMCF_MGI_SMPID_VALUE) == current_count;
}

static bool smcf_data_is_sample_valid_after_copy(
    const struct data_header header,
    const uint32_t current_count)
{
    return (is_header_include_count_id(header.format)) &&
            is_header_include_end_id(header.format) ?
        is_count_id_match(*header.start_count_id_addr, current_count) &&
            is_count_id_match(*header.end_count_id_addr, current_count) :
        true;
}

uint32_t smcf_data_get_tag_length(const struct data_header header)
{
    if (!is_tag_state_valid(header)) {
        return 0;
    }

    return (*header.start_tag_length_addr) & SMCF_MGI_SMPID_VALUE;
}

static int smcf_data_copy_tag(
    const struct data_header header,
    uint32_t *const sample_tag)
{
    uint32_t header_tag_length;
    uint32_t tag_index;

    if (!is_tag_state_valid(header)) {
        return FWK_E_STATE;
    }

    header_tag_length = smcf_data_get_tag_length(header);

    for (tag_index = 0; tag_index < header_tag_length; tag_index++) {
        sample_tag[tag_index] = *(header.start_tag_addr + tag_index);
    }

    return FWK_SUCCESS;
}

int smcf_data_get_data(
    const struct smcf_data_attr data_attributes,
    const uint32_t mli_index,
    uint32_t *const data_dest_addr,
    uint32_t *const tag_dest_addr)
{
    uint32_t count_id;

    if (!smcf_data_is_sample_valid_before_copy(
            data_attributes.header, mli_index)) {
        return FWK_E_STATE;
    }

    count_id = get_start_sample_id_value(data_attributes.header);

    if (tag_dest_addr != NULL) {
        smcf_data_copy_tag(data_attributes.header, tag_dest_addr);
    }

    smcf_copy_data(data_attributes, mli_index, data_dest_addr);

    if (!smcf_data_is_sample_valid_after_copy(
            data_attributes.header, count_id)) {
        return FWK_E_STATE;
    }

    return FWK_SUCCESS;
}

uint32_t smcf_data_get_group_id(const struct smcf_data_attr data_attributes)
{
    return (is_header_include_group_id(data_attributes.header.format)) ?
        (*data_attributes.header.group_id_addr & SMCF_GPR_ID_GRP_ID) :
        0;
}
