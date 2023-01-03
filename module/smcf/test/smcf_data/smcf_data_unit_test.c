/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "stdint.h"
#include "unity.h"

#include <Mockmgi.h>
#include <strings.h>

#include UNIT_TEST_SRC

#define DATA_PATTERN_64_BITS 0x6464646464646400
#define DATA_PATTERN_48_BITS 0x48484848484800
#define DATA_PATTERN_32_BITS 0x32323200
#define DATA_PATTERN_24_BITS 0x242400
#define DATA_PATTERN_16_BITS 0x1600
#define DATA_PATTERN_8_BITS  0x80

void setUp(void)
{
}

void tearDown(void)
{
}

void utest_smcf_data_get_data_buffer_size_2_64_bit(void)
{
    struct smcf_data_attr data_attr;

    data_attr.num_of_data = 2;
    data_attr.data_width = 64;

    TEST_ASSERT_EQUAL(4, smcf_data_get_data_buffer_size(data_attr));
}

void utest_smcf_data_get_data_buffer_size_4_32_bit(void)
{
    struct smcf_data_attr data_attr;

    data_attr.num_of_data = 4;
    data_attr.data_width = 32;

    TEST_ASSERT_EQUAL(4, smcf_data_get_data_buffer_size(data_attr));
}

void utest_smcf_data_get_data_buffer_size_4_16_bit(void)
{
    struct smcf_data_attr data_attr;

    data_attr.num_of_data = 4;
    data_attr.data_width = 16;

    TEST_ASSERT_EQUAL(4, smcf_data_get_data_buffer_size(data_attr));
}

void populate_mgi_data_width_greater_than_32bit(
    const unsigned int num_items,
    uint64_t pattern,
    uint64_t *data)
{
    unsigned int pattern_idx;

    for (pattern_idx = 0; pattern_idx < num_items; pattern_idx++) {
        data[pattern_idx] = (pattern + pattern_idx);
    }
}

void populate_mgi_data_width_equal_32bit(
    const unsigned int num_items,
    uint32_t pattern,
    uint32_t *data)
{
    unsigned int pattern_idx;

    for (pattern_idx = 0; pattern_idx < num_items; pattern_idx++) {
        data[pattern_idx] = (pattern + pattern_idx);
    }
}

void populate_hardware_and_expectation_data_width_less_than_32bit(
    const unsigned int num_items,
    const unsigned int num_mli,
    uint32_t pattern,
    uint32_t *expected_data,
    uint32_t *data_in_hardware)
{
    unsigned int pattern_idx;

    for (pattern_idx = 0; pattern_idx < (num_items * num_mli); pattern_idx++) {
        expected_data[pattern_idx] = (pattern + pattern_idx);
        data_in_hardware[pattern_idx] = expected_data[pattern_idx];
    }
}

void populate_paked_hardware_data_and_expectation_data(
    const unsigned int num_items,
    const unsigned int num_mli,
    uint32_t num_of_bits,
    uint32_t pattern,
    uint32_t *expected_data,
    uint32_t *data_in_hardware)
{
    uint32_t shift;
    unsigned int data_idx;
    unsigned int pattern_idx;
    uint32_t num_of_sections_in_32bits = 32 / num_of_bits;

    /* Clear the data*/
    bzero(
        data_in_hardware,
        num_items * num_mli * sizeof(uint32_t) / num_of_sections_in_32bits);

    for (pattern_idx = 0; pattern_idx < num_items * num_mli; pattern_idx++) {
        shift = (pattern_idx % num_of_sections_in_32bits) * num_of_bits;
        data_idx = pattern_idx / num_of_sections_in_32bits;
        expected_data[pattern_idx] = (pattern + pattern_idx);
        data_in_hardware[data_idx] |= (expected_data[pattern_idx] << shift);
    }
}

void utest_smcf_copy_data_mli0_1_64_bit(void)
{
    uint32_t pattern[2] = { 0xDEADBEEF, 0xF005BA11 };
    const unsigned int num_items = 2;
    unsigned int mli_idx = 0;
    uint32_t buffer[2];
    struct smcf_data_attr data_attr = {
        .data_addr = pattern,
        .num_of_data = 1,
        .data_width = 64,
    };

    smcf_copy_data(data_attr, mli_idx, buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(pattern, buffer, num_items);
}

void utest_smcf_copy_data_mli0_2_64_bit(void)
{
    const unsigned int num_items = 2;
    unsigned int mli_idx = 0;
    uint32_t buffer[num_items * 2];
    uint64_t pattern[num_items];
    struct smcf_data_attr data_attr = {
        .data_addr = (uint32_t volatile const *)pattern,
        .num_of_data = 2,
        .data_width = 64,
    };

    populate_mgi_data_width_greater_than_32bit(
        num_items, DATA_PATTERN_64_BITS, pattern);

    smcf_copy_data(data_attr, mli_idx, buffer);

    TEST_ASSERT_EQUAL_HEX64_ARRAY(pattern, buffer, num_items);
}

void utest_smcf_copy_data_mli0_1_48_bit(void)
{
    const unsigned int num_items = 1;
    unsigned int mli_idx = 0;
    uint32_t buffer[num_items * 2];
    uint64_t pattern[num_items];
    struct smcf_data_attr data_attr = {
        .data_addr = (uint32_t volatile const *)pattern,
        .num_of_data = num_items,
        .data_width = 48,
    };

    populate_mgi_data_width_greater_than_32bit(
        num_items, DATA_PATTERN_48_BITS, pattern);

    smcf_copy_data(data_attr, mli_idx, buffer);

    TEST_ASSERT_EQUAL_HEX64_ARRAY(pattern, buffer, num_items);
}

void utest_smcf_copy_data_mli0_1_32_bit(void)
{
    const unsigned int num_items = 1;
    unsigned int mli_idx = 0;
    uint32_t buffer[num_items];
    uint32_t pattern[num_items];
    struct smcf_data_attr data_attr = {
        .data_addr = pattern,
        .num_of_data = num_items,
        .data_width = 32,
    };

    populate_mgi_data_width_equal_32bit(
        num_items, DATA_PATTERN_32_BITS, pattern);

    smcf_copy_data(data_attr, mli_idx, buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(pattern, buffer, num_items);
}

void utest_smcf_copy_data_mli1_1_32_bit(void)
{
    const unsigned int num_items = 1;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t buffer[num_items];
    uint32_t pattern[num_items * num_mli];
    struct smcf_data_attr data_attr = {
        .data_addr = pattern,
        .num_of_data = num_items,
        .data_width = 32,
    };

    populate_mgi_data_width_equal_32bit(
        num_items, DATA_PATTERN_32_BITS, pattern);

    smcf_copy_data(data_attr, mli_idx, buffer);

    TEST_ASSERT_EQUAL_HEX32(pattern[mli_idx], buffer[0]);
}

void utest_smcf_copy_data_mli1_2_32_bit(void)
{
    const unsigned int num_items = 2;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t buffer[num_items];
    uint32_t pattern[num_items * num_mli];
    struct smcf_data_attr data_attr = {
        .data_addr = pattern,
        .num_of_data = num_items,
        .data_width = 32,
    };

    populate_mgi_data_width_equal_32bit(
        num_items, DATA_PATTERN_32_BITS, pattern);

    smcf_copy_data(data_attr, mli_idx, buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &pattern[mli_idx * num_items], buffer, num_items);
}

void utest_smcf_copy_data_mli0_1_24_bit(void)
{
    const unsigned int num_items = 1;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[num_items];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 24,
        .packed = false,
    };

    populate_hardware_and_expectation_data_width_less_than_32bit(
        num_items, num_mli, DATA_PATTERN_24_BITS, expected_data, hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_copy_data_mli0_1_16_bit(void)
{
    const unsigned int num_items = 1;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[num_items];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = false,
    };

    populate_hardware_and_expectation_data_width_less_than_32bit(
        num_items, num_mli, DATA_PATTERN_16_BITS, expected_data, hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_copy_data_mli0_3_16_bit(void)
{
    const unsigned int num_items = 3;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t out_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[num_items];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = false,
    };

    populate_hardware_and_expectation_data_width_less_than_32bit(
        num_items, num_mli, DATA_PATTERN_16_BITS, expected_data, hardware_data);

    smcf_copy_data(data_attr, mli_idx, out_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], out_buffer, num_items);
}

void utest_smcf_copy_data_mli0_1_16_bit_packed(void)
{
    const unsigned int num_items = 1;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 0;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[num_items * num_mli / 2];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = true,
    };

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        16,
        DATA_PATTERN_16_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_copy_data_mli0_2_16_bit_packed(void)
{
    const unsigned int num_items = 2;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 0;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[num_items * num_mli / 2];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = true,
    };

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        16,
        DATA_PATTERN_16_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_copy_data_mli1_2_16_bit_packed(void)
{
    const unsigned int num_items = 2;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[num_items * num_mli / 2];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = true,
    };

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        16,
        DATA_PATTERN_16_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_copy_data_mli2_3_16_bit_packed(void)
{
    const unsigned int num_items = 3;
    const unsigned int num_mli = 3;
    unsigned int mli_idx = 2;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[(num_items * num_mli)];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = true,
    };

    /* Data layout in the hardware buffer
     *
     * This MGI contains 3 MLI's: mli[0], mli[1] and mli[2]
     * Each MLI provides 3 samples. For example mli[0] provides:
     * mli[0][0], mli[0][1] and mli[0][2]
     *
     * Because this test-case is testing data for mli[2], only mli[2] data
     * is populated with this pattern.
     *
     *    0______8_______16______24_______32
     *[0] |__ mli[0][0]__ |__mli[0][1]___ |
     *[1] |___mli[0][2]__ |__mli[1][0]___ |
     *[2] |___mli[1][1]__ |__mli[1][2]___ |
     *[3] |     0x1606    |     0x1607    |
     *[4] |     0x1608    |               |
     *    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     */
    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        16,
        DATA_PATTERN_16_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_copy_data_mli3_3_16_bit_packed(void)
{
    const unsigned int num_items = 3;
    const unsigned int num_mli = 4;
    unsigned int mli_idx = 3;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[(num_items * num_mli)];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 16,
        .packed = true,
    };

    /* Data layout in the hardware buffer
     *
     *    0______8_______16______24_______32
     *[0] |__ mli[0][0]__ |__mli[0][1]___ |
     *[1] |___mli[0][2]__ |__mli[1][0]___ |
     *[2] |___mli[1][1]__ |__mli[1][2]___ |
     *[3] |___mli[2][0]__ |__mli[2][1]___ |
     *[4] |___mli[2][2]__ |    0x1609     |
     *[5] |   0x160A      |    0x160B     |
     *    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     */

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        16,
        DATA_PATTERN_16_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_data_sample_width_1_to_8_packed_num_data_4(void)
{
    const unsigned int num_items = 4;
    const unsigned int num_mli = 1;
    unsigned int mli_idx = 0;

    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[(num_items * num_mli)];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 8,
        .packed = true,
    };

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        8,
        DATA_PATTERN_8_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_data_sample_width_1_to_8_packed_num_data_3(void)
{
    const unsigned int num_items = 3;
    const unsigned int num_mli = 2;
    unsigned int mli_idx = 1;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[(num_items * num_mli)];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 8,
        .packed = true,
    };

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        8,
        DATA_PATTERN_8_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_data_sample_width_1_to_8_packed_num_data_5(void)
{
    const unsigned int num_items = 5;
    const unsigned int num_mli = 7;
    unsigned int mli_idx = 3;
    uint32_t output_buffer[num_items];
    uint32_t expected_data[num_items * num_mli];
    uint32_t hardware_data[(num_items * num_mli)];
    struct smcf_data_attr data_attr = {
        .data_addr = hardware_data,
        .num_of_data = num_items,
        .data_width = 8,
        .packed = true,
    };

    populate_paked_hardware_data_and_expectation_data(
        num_items,
        num_mli,
        8,
        DATA_PATTERN_8_BITS,
        expected_data,
        hardware_data);

    smcf_copy_data(data_attr, mli_idx, output_buffer);

    TEST_ASSERT_EQUAL_HEX32_ARRAY(
        &expected_data[mli_idx * num_items], output_buffer, num_items);
}

void utest_smcf_data_copy_tag(void)
{
    uint32_t const sample_tag_length = 4;
    uint32_t FWK_R hw_tag_length = /* (tag lenght) + tag length value */
        (9 << 28) + sample_tag_length;
    uint32_t sample_tag[sample_tag_length];
    uint32_t expected_sample_tag[sample_tag_length];
    struct data_header data_header;
    uint32_t tag_index;
    int status;

    data_header.format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID;
    data_header.start_tag_addr = expected_sample_tag;
    data_header.start_tag_length_addr = &hw_tag_length;

    for (tag_index = 0; tag_index < sample_tag_length; tag_index++) {
        expected_sample_tag[tag_index] = tag_index + 3;
    }

    status = smcf_data_copy_tag(data_header, sample_tag);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(
        expected_sample_tag, sample_tag, sample_tag_length);
}

void utest_smcf_sample_header_get_group_id_not_supported(void)
{
    struct smcf_data_attr data_attributes;
    uint32_t group_id;

    data_attributes.header.format = 0;

    group_id = smcf_data_get_group_id(data_attributes);

    TEST_ASSERT_EQUAL(0, group_id);
}

void utest_smcf_sample_header_get_group_id(void)
{
    struct smcf_data_attr data_attributes;
    uint32_t expected_group_id = 0xAA;
    uint32_t group_id = 0;

    data_attributes.header.format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID;
    data_attributes.header.group_id_addr = &expected_group_id;

    group_id = smcf_data_get_group_id(data_attributes);

    TEST_ASSERT_EQUAL(expected_group_id, group_id);
}

void utest_smcf_set_data_address_mgi_error_address(void)
{
    int status = FWK_SUCCESS;

    status = smcf_set_data_address_mgi(0, 0);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_set_data_address_mgi_set_address(void)
{
    struct smcf_mgi_reg mgi = { .DATA[0] = 0xFF, .DATA[1] = 0xAA };
    struct smcf_data_attr data_attr;
    int status = FWK_SUCCESS;

    status = smcf_set_data_address_mgi(&mgi, &data_attr);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(&mgi.DATA, data_attr.data_addr);
    TEST_ASSERT_EQUAL_HEX32(0xFF, *data_attr.data_addr);
    TEST_ASSERT_EQUAL_HEX32(0xAA, *(data_attr.data_addr + 1));
}

void utest_smcf_set_data_header_address_mgi_error_address(void)
{
    int status = FWK_SUCCESS;

    status = smcf_set_data_address_mgi(0, 0);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_set_data_header_address_mgi_set_address(void)
{
    struct smcf_mgi_reg mgi = {
        .GRP_ID = 0x05,
    };
    struct smcf_data_attr data_attr;
    int status = FWK_SUCCESS;

    status = smcf_set_data_address_mgi(&mgi, &data_attr);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    TEST_ASSERT_EQUAL_PTR(&mgi.GRP_ID, data_attr.header.group_id_addr);
    TEST_ASSERT_EQUAL_PTR(&mgi.DVLD, data_attr.header.valid_bits_addr);
    TEST_ASSERT_EQUAL_PTR(
        &mgi.SMPID_START, data_attr.header.start_count_id_addr);
    TEST_ASSERT_EQUAL_PTR(&mgi.SMPID_END, data_attr.header.end_count_id_addr);
    TEST_ASSERT_EQUAL_PTR(&mgi.TAG0, data_attr.header.start_tag_addr);
    TEST_ASSERT_EQUAL_PTR(&mgi.TAG0, data_attr.header.end_tag_addr);
    TEST_ASSERT_EQUAL_HEX32(0x05, *data_attr.header.group_id_addr);
}

void utest_smcf_sample_header_is_group_id_available_yes(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID;
    bool group_id_available = false;

    group_id_available = is_header_include_group_id(header_format);

    TEST_ASSERT_TRUE(group_id_available);
}

void utest_smcf_sample_header_is_group_id_available_no(void)
{
    uint32_t header_format = 0;
    bool group_id_available = true;

    group_id_available = is_header_include_group_id(header_format);

    TEST_ASSERT_FALSE(group_id_available);
}

void utest_smcf_sample_header_is_valid_bits_available_yes(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS;
    bool data_valid_available = false;

    data_valid_available = is_header_include_valid_bits(header_format);

    TEST_ASSERT_TRUE(data_valid_available);
}

void utest_smcf_sample_header_is_valid_bits_available_no(void)
{
    uint32_t header_format = 0;
    bool data_valid_available = true;

    data_valid_available = is_header_include_valid_bits(header_format);

    TEST_ASSERT_FALSE(data_valid_available);
}

void utest_smcf_sample_header_is_count_id_available_yes(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID;
    bool count_id_available = false;

    count_id_available = is_header_include_count_id(header_format);

    TEST_ASSERT_TRUE(count_id_available);
}

void utest_smcf_sample_header_is_count_id_available_no(void)
{
    uint32_t header_format = 0;
    bool count_id_available = true;

    count_id_available = is_header_include_count_id(header_format);

    TEST_ASSERT_FALSE(count_id_available);
}

void utest_smcf_sample_header_is_tag_available_yes(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID;
    bool tag_available = false;

    tag_available = is_header_include_tag_id(header_format);

    TEST_ASSERT_TRUE(tag_available);
}

void utest_smcf_sample_header_is_tag_available_no(void)
{
    uint32_t header_format = 0;
    bool tag_available = true;

    tag_available = is_header_include_tag_id(header_format);

    TEST_ASSERT_FALSE(tag_available);
}

void utest_smcf_sample_header_is_end_id_available_yes(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_END_ID;
    bool end_id_available = false;

    end_id_available = is_header_include_end_id(header_format);

    TEST_ASSERT_TRUE(end_id_available);
}

void utest_smcf_sample_header_is_end_id_available_no(void)
{
    uint32_t header_format = 0;
    bool end_id_available = true;

    end_id_available = is_header_include_end_id(header_format);

    TEST_ASSERT_FALSE(end_id_available);
}

void utest_smcf_data_include_header_group_id(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(1, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.group_id_addr);
}

void utest_smcf_data_include_header_valid_bits(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(1, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.valid_bits_addr);
}

void utest_smcf_data_include_header_count_id(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(1, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.start_count_id_addr);
}

void utest_smcf_data_include_header_tag_id(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_tag_id_write_to_ram_ExpectAnyArgs();
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(4, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.start_tag_length_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.start_tag_addr);
}

void utest_smcf_data_include_header_start_and_end_count_id(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(2, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.start_count_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.end_count_id_addr);
}

void utest_smcf_data_include_header_start_and_end_tag_id(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format =
            SMCF_SAMPLE_HEADER_FORMAT_TAG_ID | SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_tag_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(6, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.start_tag_length_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.start_tag_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 3, data_attr.header.end_tag_length_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 4, data_attr.header.end_tag_addr);
}

void utest_smcf_data_include_header_group_and_valid_bits(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
            SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();
    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(2, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.group_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.valid_bits_addr);
}

void utest_smcf_data_include_header_group_valid_bits_count_id(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
            SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
            SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();
    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();
    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(3, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.group_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.valid_bits_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 2, data_attr.header.start_count_id_addr);
}

void utest_smcf_data_include_header_group_valid_bits_count_id_tag(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
            SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
            SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_TAG_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();
    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();
    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_enable_tag_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(7, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.group_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.valid_bits_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 2, data_attr.header.start_count_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 3, data_attr.header.start_tag_length_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 4, data_attr.header.start_tag_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 6, data_attr.header.end_tag_length_addr);
}

void utest_smcf_data_include_header_group_valid_bits_count_id_tag_end(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
            SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
            SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_TAG_ID | SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    unsigned int addr_offset = 0;
    struct smcf_mgi_reg mgi;

    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();
    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();
    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_enable_tag_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);

    addr_offset = smcf_data_include_header(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(10, addr_offset);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr, data_attr.header.group_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 1, data_attr.header.valid_bits_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 2, data_attr.header.start_count_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 3, data_attr.header.end_count_id_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 4, data_attr.header.start_tag_length_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 5, data_attr.header.start_tag_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 7, data_attr.header.end_tag_length_addr);
    TEST_ASSERT_EQUAL_PTR(
        data_config.read_addr + 8, data_attr.header.end_tag_addr);
}

void utest_get_number_of_32_bits_in_8bits(void)
{
    uint32_t number_of_words = get_number_of_32_bits(8);

    TEST_ASSERT_EQUAL(1, number_of_words);
}

void utest_get_number_of_32_bits_in_27bits(void)
{
    uint32_t number_of_words = get_number_of_32_bits(27);

    TEST_ASSERT_EQUAL(1, number_of_words);
}

void utest_get_number_of_32_bits_in_32bits(void)
{
    uint32_t number_of_words = get_number_of_32_bits(32);

    TEST_ASSERT_EQUAL(1, number_of_words);
}

void utest_get_number_of_32_bits_in_64bits(void)
{
    uint32_t number_of_words = get_number_of_32_bits(64);

    TEST_ASSERT_EQUAL(2, number_of_words);
}

void utest_get_number_of_32_bits_in_77bits(void)
{
    uint32_t number_of_words = get_number_of_32_bits(77);

    TEST_ASSERT_EQUAL(3, number_of_words);
}

void utest_smcf_set_data_address_ram(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
            SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
            SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_TAG_ID | SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    struct smcf_data_attr data_attr;
    struct smcf_mgi_reg mgi;
    int status;

    mgi_is_dma_supported_ExpectAnyArgsAndReturn(true);
    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();
    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();
    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_enable_tag_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_set_dma_data_address_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mgi_enable_dma_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = smcf_set_data_address_ram(&mgi, data_config, &data_attr);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(data_config.read_addr + 10, data_attr.data_addr);
}

void utest_smcf_data_header_tag_requested_not_supported(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID;
    struct smcf_mgi_reg mgi;
    int status;

    mgi_is_tag_supported_ExpectAnyArgsAndReturn(false);

    status = smcf_validate_header_format(&mgi, header_format);

    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_smcf_data_header_tag_requested_supported(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID;
    struct smcf_mgi_reg mgi;
    int status;

    mgi_is_tag_supported_ExpectAnyArgsAndReturn(true);

    status = smcf_validate_header_format(&mgi, header_format);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_data_header_end_id_requested_no_count_id_no_tag(void)
{
    uint32_t header_format = SMCF_SAMPLE_HEADER_FORMAT_END_ID;
    struct smcf_mgi_reg mgi;
    int status;

    mgi_is_tag_supported_ExpectAnyArgsAndReturn(true);

    status = smcf_validate_header_format(&mgi, header_format);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_data_header_end_id_requested_and_count_id(void)
{
    uint32_t header_format =
        SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID | SMCF_SAMPLE_HEADER_FORMAT_END_ID;
    struct smcf_mgi_reg mgi;
    int status;

    mgi_is_tag_supported_ExpectAnyArgsAndReturn(true);

    status = smcf_validate_header_format(&mgi, header_format);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_data_header_end_id_requested_and_tag(void)
{
    uint32_t header_format =
        SMCF_SAMPLE_HEADER_FORMAT_TAG_ID | SMCF_SAMPLE_HEADER_FORMAT_END_ID;
    struct smcf_mgi_reg mgi;
    int status;

    mgi_is_tag_supported_ExpectAnyArgsAndReturn(true);

    status = smcf_validate_header_format(&mgi, header_format);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_set_data_address_valid_location(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID,
        .data_location = SMCF_DATA_LOCATION_MGI,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    int status;
    struct smcf_data_attr data_attr;
    struct smcf_mgi_reg mgi = { .DATA[0] = 0xFF, .DATA[1] = 0xAA };

    status = smcf_data_set_data_address(&mgi, data_config, &data_attr);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    /* Set data location to SCMF RAM */
    data_config.data_location = SMCF_DATA_LOCATION_RAM;
    mgi_is_dma_supported_ExpectAnyArgsAndReturn(true);
    mgi_enable_group_id_write_to_ram_ExpectAnyArgs();
    mgi_enable_valid_bits_write_to_ram_ExpectAnyArgs();
    mgi_enable_count_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_enable_tag_id_write_to_ram_ExpectAnyArgs();
    mgi_request_start_id_wirte_to_ram_ExpectAnyArgs();
    mgi_request_start_and_end_id_wirte_to_ram_ExpectAnyArgs();
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_get_tag_length_in_bits_ExpectAnyArgsAndReturn(33);
    mgi_set_dma_data_address_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mgi_enable_dma_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = smcf_data_set_data_address(&mgi, data_config, &data_attr);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_set_data_address_invalid_location(void)
{
    struct mod_smcf_data_config data_config = {
        .header_format = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID,
        .data_location = SMCF_DATA_LOCATION_ALTERNATE,
        .read_addr = (uint32_t *)0x55AA55AA,
    };
    int status;
    struct smcf_data_attr data_attr;
    struct smcf_mgi_reg mgi = { .DATA[0] = 0xFF, .DATA[1] = 0xAA };

    status = smcf_data_set_data_address(&mgi, data_config, &data_attr);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_smcf_sample_valid_before_copy_no_valid_bits(void)
{
    bool is_available = true;
    unsigned int mli_index = 0;
    uint32_t FWK_R valid_bits = 0x0;
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS,
        .valid_bits_addr = &valid_bits,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_FALSE(is_available);
}

void utest_smcf_sample_valid_before_copy_yes_valid_bits_not_supported(void)
{
    bool is_available = false;
    unsigned int mli_index = 0;
    struct data_header header = {
        .format = 0,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_TRUE(is_available);
}

void utest_smcf_sample_valid_before_copy_yes_valid_bits_mli0(void)
{
    bool is_available = false;
    unsigned int mli_index = 0;
    uint32_t FWK_R valid_bits = 0x1;
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS,
        .valid_bits_addr = &valid_bits,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_TRUE(is_available);
}

void utest_smcf_sample_valid_before_copy_yes_valid_bits_mli32(void)
{
    bool is_available = false;
    unsigned int mli_index = 32;
    uint32_t FWK_R valid_bits = 0x1 << mli_index;
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS,
        .valid_bits_addr = &valid_bits,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_TRUE(is_available);
}

void utest_smcf_sample_valid_before_copy_count_id_no_status_power_off(void)
{
    bool is_available = true;
    unsigned int mli_index = 0;
    uint32_t FWK_R start_count = 1 << 28; /* status powered off */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID,
        .start_count_id_addr = &start_count,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_FALSE(is_available);
}

void utest_smcf_sample_valid_before_copy_tag_no_status_power_off(void)
{
    bool is_available = true;
    unsigned int mli_index = 0;
    uint32_t FWK_R tag_length = 1 << 28; /* status powered off */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID,
        .start_tag_length_addr = &tag_length,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_FALSE(is_available);
}

void utest_smcf_sample_valid_before_copy_no_start_end_count_mismatch(void)
{
    bool is_available = true;
    unsigned int mli_index = 0;
    uint32_t FWK_R start_count = (0x8 << 28) + 4; /* (count id) + start val */
    uint32_t FWK_R end_count = (0x8 << 28) + 3; /* (count id) + end val */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .start_count_id_addr = &start_count,
        .end_count_id_addr = &end_count,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_FALSE(is_available);
}

void utest_smcf_sample_valid_before_copy_yes_start_end_count_match(void)
{
    bool is_available = false;
    unsigned int mli_index = 0;
    uint32_t FWK_R start_count = (0x8 << 28) + 4; /* (count id) + start val */
    uint32_t FWK_R end_count = (0x8 << 28) + 4; /* (count id) + end val */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .start_count_id_addr = &start_count,
        .end_count_id_addr = &end_count,
    };

    is_available = smcf_data_is_sample_valid_before_copy(header, mli_index);

    TEST_ASSERT_TRUE(is_available);
}

void utest_smcf_sample_valid_after_copy_no_start_count_mismatch(void)
{
    bool is_available = true;
    uint32_t current_count = 4;
    uint32_t FWK_R start_count = (0x8 << 28) + 5; /* (count id) + end val */
    uint32_t FWK_R end_count = (0x8 << 28) + 4; /* (count id) + end val */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .start_count_id_addr = &start_count,
        .end_count_id_addr = &end_count,
    };

    is_available = smcf_data_is_sample_valid_after_copy(header, current_count);

    TEST_ASSERT_FALSE(is_available);
}

void utest_smcf_sample_valid_after_copy_no_end_count_mismatch(void)
{
    bool is_available = true;
    uint32_t current_count = 4;
    uint32_t FWK_R start_count = (0x8 << 28) + 5; /* (count id) + end val */
    uint32_t FWK_R end_count = (0x8 << 28) + 5; /* (count id) + end val */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .start_count_id_addr = &start_count,
        .end_count_id_addr = &end_count,
    };

    is_available = smcf_data_is_sample_valid_after_copy(header, current_count);

    TEST_ASSERT_FALSE(is_available);
}

void utest_smcf_sample_valid_after_copy_yes_start_end_count_match(void)
{
    bool is_available = true;
    uint32_t current_count = 5;
    uint32_t FWK_R start_count = (0x8 << 28) + 5; /* (count id) + end val */
    uint32_t FWK_R end_count = (0x8 << 28) + 5; /* (count id) + end val */
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
            SMCF_SAMPLE_HEADER_FORMAT_END_ID,
        .start_count_id_addr = &start_count,
        .end_count_id_addr = &end_count,
    };

    is_available = smcf_data_is_sample_valid_after_copy(header, current_count);

    TEST_ASSERT_TRUE(is_available);
}

void utest_smcf_data_get_data_fail_mgi_off(void)
{
    int status = FWK_SUCCESS;
    uint32_t mli_index = 0;
    uint32_t buffer;
    uint32_t tag;
    uint32_t count_id = (0x1 << 28); /* status off */
    struct smcf_data_attr data_attr = {
        .header = {
            .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID,
            .start_count_id_addr = &count_id,
        },
    };

    status = smcf_data_get_data(data_attr, mli_index, &buffer, &tag);

    TEST_ASSERT_EQUAL(FWK_E_STATE, status);
}

void utest_smcf_data_get_data_success_start_count_id(void)
{
    int status = FWK_E_STATE;
    uint32_t mli_index = 0;
    uint32_t buffer;
    uint32_t tag;
    uint32_t count_id = (0x8 << 28) + 5; /* (count id) + count val */
    struct smcf_data_attr data_attr = {
        .header = {
            .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID,
            .start_count_id_addr = &count_id,
        },
    };

    status = smcf_data_get_data(data_attr, mli_index, &buffer, &tag);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_data_get_data_success_copy_data(void)
{
    int status = FWK_E_STATE;
    uint32_t mli_index = 0;
    uint32_t data[2] = { 0xAA, 0x55 };
    uint32_t buffer[2];
    uint32_t tag;
    struct smcf_data_attr data_attr = {
        .data_addr = data,
        .num_of_data = 1,
        .data_width = 32,
    };

    status = smcf_data_get_data(data_attr, mli_index, buffer, &tag);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(0XAA, buffer[0]);
    TEST_ASSERT_NOT_EQUAL(0X55, buffer[1]);
}

void utest_smcf_data_get_data_success_end_count_id(void)
{
    int status = FWK_E_STATE;
    uint32_t mli_index = 0;
    uint32_t buffer;
    uint32_t tag;
    uint32_t count_id = (0x8 << 28) + 5; /* (count id) + count val */
    struct smcf_data_attr data_attr = {
        .header = {
            .format = SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID |
                      SMCF_SAMPLE_HEADER_FORMAT_END_ID,
            .start_count_id_addr = &count_id,
            .end_count_id_addr = &count_id,
        },
    };

    status = smcf_data_get_data(data_attr, mli_index, &buffer, &tag);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_data_get_tag_length_not_supported(void)
{
    struct data_header header = {
        .format = 0,
    };
    uint32_t tag_length;

    tag_length = smcf_data_get_tag_length(header);

    TEST_ASSERT_EQUAL(tag_length, 0);
}

void utest_smcf_data_get_tag_length_correct_value(void)
{
    uint32_t FWK_R hw_tag_length =
        /* (tag lenght) + tag length value */
        (9 << 28) + 4;
    struct data_header header = {
        .format = SMCF_SAMPLE_HEADER_FORMAT_TAG_ID,
        .start_tag_length_addr = &hw_tag_length,
    };
    uint32_t tag_length;

    tag_length = smcf_data_get_tag_length(header);

    TEST_ASSERT_EQUAL(tag_length, 4);
}

int smcf_data_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(utest_smcf_data_get_data_buffer_size_2_64_bit);
    RUN_TEST(utest_smcf_data_get_data_buffer_size_4_32_bit);
    RUN_TEST(utest_smcf_data_get_data_buffer_size_4_16_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_1_64_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_2_64_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_1_48_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_1_32_bit);
    RUN_TEST(utest_smcf_copy_data_mli1_1_32_bit);
    RUN_TEST(utest_smcf_copy_data_mli1_2_32_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_1_24_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_1_16_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_3_16_bit);
    RUN_TEST(utest_smcf_copy_data_mli0_1_16_bit_packed);
    RUN_TEST(utest_smcf_copy_data_mli0_2_16_bit_packed);
    RUN_TEST(utest_smcf_copy_data_mli1_2_16_bit_packed);
    RUN_TEST(utest_smcf_copy_data_mli2_3_16_bit_packed);
    RUN_TEST(utest_smcf_copy_data_mli3_3_16_bit_packed);
    RUN_TEST(utest_smcf_data_sample_width_1_to_8_packed_num_data_4);
    RUN_TEST(utest_smcf_data_sample_width_1_to_8_packed_num_data_3);
    RUN_TEST(utest_smcf_data_sample_width_1_to_8_packed_num_data_5);
    RUN_TEST(utest_smcf_data_copy_tag);
    RUN_TEST(utest_smcf_sample_header_get_group_id_not_supported);
    RUN_TEST(utest_smcf_sample_header_get_group_id);
    RUN_TEST(utest_smcf_set_data_address_valid_location);
    RUN_TEST(utest_smcf_set_data_address_invalid_location);
    RUN_TEST(utest_smcf_set_data_address_mgi_error_address);
    RUN_TEST(utest_smcf_set_data_address_mgi_set_address);
    RUN_TEST(utest_smcf_set_data_header_address_mgi_error_address);
    RUN_TEST(utest_smcf_set_data_header_address_mgi_set_address);
    RUN_TEST(utest_smcf_sample_header_is_group_id_available_yes);
    RUN_TEST(utest_smcf_sample_header_is_group_id_available_no);
    RUN_TEST(utest_smcf_sample_header_is_valid_bits_available_yes);
    RUN_TEST(utest_smcf_sample_header_is_valid_bits_available_no);
    RUN_TEST(utest_smcf_sample_header_is_count_id_available_yes);
    RUN_TEST(utest_smcf_sample_header_is_count_id_available_no);
    RUN_TEST(utest_smcf_sample_header_is_tag_available_yes);
    RUN_TEST(utest_smcf_sample_header_is_tag_available_no);
    RUN_TEST(utest_smcf_sample_header_is_end_id_available_yes);
    RUN_TEST(utest_smcf_sample_header_is_end_id_available_no);
    RUN_TEST(utest_smcf_data_include_header_group_id);
    RUN_TEST(utest_smcf_data_include_header_valid_bits);
    RUN_TEST(utest_smcf_data_include_header_count_id);
    RUN_TEST(utest_smcf_data_include_header_tag_id);
    RUN_TEST(utest_smcf_data_include_header_start_and_end_count_id);
    RUN_TEST(utest_smcf_data_include_header_start_and_end_tag_id);
    RUN_TEST(utest_smcf_data_include_header_group_and_valid_bits);
    RUN_TEST(utest_smcf_data_include_header_group_valid_bits_count_id);
    RUN_TEST(utest_smcf_data_include_header_group_valid_bits_count_id_tag);
    RUN_TEST(utest_smcf_data_include_header_group_valid_bits_count_id_tag_end);
    RUN_TEST(utest_get_number_of_32_bits_in_8bits);
    RUN_TEST(utest_get_number_of_32_bits_in_27bits);
    RUN_TEST(utest_get_number_of_32_bits_in_32bits);
    RUN_TEST(utest_get_number_of_32_bits_in_64bits);
    RUN_TEST(utest_get_number_of_32_bits_in_77bits);
    RUN_TEST(utest_smcf_data_header_tag_requested_not_supported);
    RUN_TEST(utest_smcf_data_header_tag_requested_supported);
    RUN_TEST(utest_smcf_data_header_end_id_requested_no_count_id_no_tag);
    RUN_TEST(utest_smcf_data_header_end_id_requested_and_count_id);
    RUN_TEST(utest_smcf_data_header_end_id_requested_and_tag);
    RUN_TEST(utest_smcf_sample_valid_before_copy_no_valid_bits);
    RUN_TEST(utest_smcf_sample_valid_before_copy_yes_valid_bits_not_supported);
    RUN_TEST(utest_smcf_sample_valid_before_copy_yes_valid_bits_mli0);
    RUN_TEST(utest_smcf_sample_valid_before_copy_yes_valid_bits_mli32);
    RUN_TEST(utest_smcf_sample_valid_before_copy_count_id_no_status_power_off);
    RUN_TEST(utest_smcf_sample_valid_before_copy_tag_no_status_power_off);
    RUN_TEST(utest_smcf_sample_valid_before_copy_no_start_end_count_mismatch);
    RUN_TEST(utest_smcf_sample_valid_before_copy_yes_start_end_count_match);
    RUN_TEST(utest_smcf_sample_valid_after_copy_no_start_count_mismatch);
    RUN_TEST(utest_smcf_sample_valid_after_copy_no_end_count_mismatch);
    RUN_TEST(utest_smcf_sample_valid_after_copy_yes_start_end_count_match);
    RUN_TEST(utest_smcf_data_get_data_fail_mgi_off);
    RUN_TEST(utest_smcf_data_get_data_success_start_count_id);
    RUN_TEST(utest_smcf_data_get_data_success_copy_data);
    RUN_TEST(utest_smcf_data_get_data_success_end_count_id);
    RUN_TEST(utest_smcf_set_data_address_ram);
    RUN_TEST(utest_smcf_data_get_tag_length_not_supported);
    RUN_TEST(utest_smcf_data_get_tag_length_correct_value);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return smcf_data_test_main();
}
#endif
