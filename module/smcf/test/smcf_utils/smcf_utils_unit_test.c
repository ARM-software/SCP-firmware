/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mod_smcf_data.h"
#include "scp_unity.h"
#include "smcf_utils.h"
#include "stdint.h"
#include "unity.h"

#include <strings.h>

#define SMCF_RAM_START_ADDRESS UINT32_C(0x51000000)
#define SMCF_TAG_LENGTH        UINT32_C(64)

/* MGI_COUNTER_SIZE: the current size of counter in bytes */
#define MGI_COUNTER_SIZE UINT32_C(8)

/* MGI_NUMBER_OF_COUNTER: the current number of counters */
#define MGI_NUMBER_OF_COUNTER UINT32_C(7)

/* MGI_PADDING: spacing between Archi and Aux counters */
#define MGI_PADDING UINT32_C(224)

#define MGI_COUNTER_DATA_SIZE \
    ((MGI_COUNTER_SIZE * MGI_NUMBER_OF_COUNTER) + MGI_PADDING)
#define MGI_SENSOR_DATA_SIZE UINT32_C(4)

#include UNIT_TEST_SRC

void setUp(void)
{
}

void tearDown(void)
{
}

void calculate_mgi_sesnsor_next_ram_offset(void)
{
    uint32_t header = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
        SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
        SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID | SMCF_SAMPLE_HEADER_FORMAT_TAG_ID |
        SMCF_SAMPLE_HEADER_FORMAT_END_ID;

    uint32_t next_ram_offces;

    next_ram_offces = get_next_smcf_ram_offset(
        SMCF_RAM_START_ADDRESS, header, MGI_SENSOR_DATA_SIZE, SMCF_TAG_LENGTH);

    TEST_ASSERT_EQUAL(next_ram_offces, (SMCF_RAM_START_ADDRESS + 0x2C));
}

void calculate_mgi_sesnsor_next_ram_offset_header_zero(void)
{
    uint32_t header = 0;

    uint32_t next_ram_offces;

    next_ram_offces = get_next_smcf_ram_offset(
        SMCF_RAM_START_ADDRESS, header, MGI_SENSOR_DATA_SIZE, SMCF_TAG_LENGTH);

    TEST_ASSERT_EQUAL(next_ram_offces, (SMCF_RAM_START_ADDRESS + 0x04));
}

void calculate_mgi_amu_next_ram_offset(void)
{
    uint32_t header = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
        SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
        SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID | SMCF_SAMPLE_HEADER_FORMAT_TAG_ID |
        SMCF_SAMPLE_HEADER_FORMAT_END_ID;

    uint32_t next_ram_offces;

    next_ram_offces = get_next_smcf_ram_offset(
        SMCF_RAM_START_ADDRESS, header, MGI_COUNTER_DATA_SIZE, SMCF_TAG_LENGTH);

    TEST_ASSERT_EQUAL(next_ram_offces, (SMCF_RAM_START_ADDRESS + 0x140));
}

void calculate_mgi_amu_next_ram_offset_header_zero(void)
{
    uint32_t header = 0;

    uint32_t next_ram_offces;

    next_ram_offces = get_next_smcf_ram_offset(
        SMCF_RAM_START_ADDRESS, header, MGI_COUNTER_DATA_SIZE, SMCF_TAG_LENGTH);

    TEST_ASSERT_EQUAL(next_ram_offces, (SMCF_RAM_START_ADDRESS + 0x118));
}

void calculate_overall_memory_consumption(void)
{
    uint32_t header = SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID |
        SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS |
        SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID | SMCF_SAMPLE_HEADER_FORMAT_TAG_ID |
        SMCF_SAMPLE_HEADER_FORMAT_END_ID;

    uint32_t next_ram_offces = SMCF_RAM_START_ADDRESS;

    for (uint8_t index = 0; index < 16; index++) {
        next_ram_offces = get_next_smcf_ram_offset(
            next_ram_offces, header, MGI_SENSOR_DATA_SIZE, SMCF_TAG_LENGTH);
    }

    for (uint8_t index = 0; index < 16; index++) {
        next_ram_offces = get_next_smcf_ram_offset(
            next_ram_offces, header, MGI_COUNTER_DATA_SIZE, SMCF_TAG_LENGTH);
    }

    TEST_ASSERT_EQUAL(next_ram_offces, (SMCF_RAM_START_ADDRESS + 0x16C0));
}

void utest_memory_align_sending_not_alligned_address_with_32bit(void)
{
    int address = 30;
    int allignement = 4;

    int result = memory_align(address, allignement);

    TEST_ASSERT_EQUAL(32, result);
}

void utest_memory_align_sending_alligned_address_with_32bit(void)
{
    int address = 20;
    int allignement = 4;

    int result = memory_align(address, allignement);

    TEST_ASSERT_EQUAL(20, result);
}

int smcf_data_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(calculate_mgi_sesnsor_next_ram_offset);
    RUN_TEST(calculate_mgi_sesnsor_next_ram_offset_header_zero);

    RUN_TEST(calculate_mgi_amu_next_ram_offset);
    RUN_TEST(calculate_mgi_amu_next_ram_offset_header_zero);

    RUN_TEST(calculate_overall_memory_consumption);

    RUN_TEST(utest_memory_align_sending_not_alligned_address_with_32bit);
    RUN_TEST(utest_memory_align_sending_alligned_address_with_32bit);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return smcf_data_test_main();
}
#endif
