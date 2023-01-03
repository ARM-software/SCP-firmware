/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "stdint.h"
#include "unity.h"

#include <fwk_assert.h>

#include UNIT_TEST_SRC

#include "mgi_unit_test.h"

struct smcf_mgi_reg fake_smcf_mgi_reg = {
    .GRP_ID = MGI_GRP_ID,
    /* Set monitor 0,5,9
     status to enabled */
    .MON_STAT = (1 << MGI_MON_IDX_MON0) | (1 << MGI_MON_IDX_MON5) |
        (1 << MGI_MON_IDX_MON9),

    .FEAT1 = (MGI_FEAT1_NUM_OF_REG) |
        (MGI_FEAT1_NUM_OF_BITS << SMCF_MGI_FEAT1_MODE_LEN_POS),
};

void setUp(void)
{
    uint32_t *ptr_set_const_reg;

    /* Clear FEAT0 */
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;
    *ptr_set_const_reg = 0;

    /* Disable all Monitors */
    fake_smcf_mgi_reg.MON_REQ = 0;
}

void tearDown(void)
{
}

void utest_mgi_get_num_of_monitors(void)
{
    uint32_t num_monitor = mgi_get_num_of_monitors(&fake_smcf_mgi_reg);
    TEST_ASSERT_EQUAL(MGI_NUM_OF_MON, num_monitor);
}

void utest_mgi_enable_monitor_one_monitor(void)
{
    uint32_t mon1_idx;
    int status;

    /* Setting one monitor */
    mon1_idx = MGI_MON_IDX_MON5;

    status = mgi_enable_monitor(&fake_smcf_mgi_reg, mon1_idx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_BIT_HIGH(mon1_idx, fake_smcf_mgi_reg.MON_REQ);
}

void utest_mgi_enable_monitor_two_monitors(void)
{
    uint32_t mon1_idx, mon2_idx;
    int status;

    /* Set first monitor */
    mon1_idx = MGI_MON_IDX_MON5;
    mgi_enable_monitor(&fake_smcf_mgi_reg, mon1_idx);

    /* Set second monitor doesn't affect the first value */
    mon2_idx = MGI_MON_IDX_MON9;

    status = mgi_enable_monitor(&fake_smcf_mgi_reg, mon2_idx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_BIT_HIGH(mon1_idx, fake_smcf_mgi_reg.MON_REQ);
    TEST_ASSERT_BIT_HIGH(mon2_idx, fake_smcf_mgi_reg.MON_REQ);
}

void utest_mgi_enable_monitor_boundary_case(void)
{
    uint32_t mon1_idx, mon2_idx;

    /* Check boundary cases */
    mon1_idx = MGI_MON_IDX_MON0;
    mgi_enable_monitor(&fake_smcf_mgi_reg, mon1_idx);

    mon2_idx = MGI_MON_IDX_MON31;
    mgi_enable_monitor(&fake_smcf_mgi_reg, mon2_idx);

    TEST_ASSERT_BIT_HIGH(mon1_idx, fake_smcf_mgi_reg.MON_REQ);
    TEST_ASSERT_BIT_HIGH(mon2_idx, fake_smcf_mgi_reg.MON_REQ);
}

void utest_mgi_enable_monitor_out_of_range(void)
{
    uint32_t mon1_idx;
    int status;

    /* Check out of range case */
    /* Note the maximum number of monitor is MGI_NUM_OF_MON */
    mon1_idx = MGI_NUM_OF_MON;

    status = mgi_enable_monitor(&fake_smcf_mgi_reg, mon1_idx);

    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_mgi_enable_all_monitor(void)
{
    /* All monitor bits in the MON_REQ register should be set to 1 */
    uint32_t full_set = (1ULL << MGI_NUM_OF_MON) - 1;
    mgi_enable_all_monitor(&fake_smcf_mgi_reg);
    TEST_ASSERT_EQUAL(full_set, fake_smcf_mgi_reg.MON_REQ);
}

void utest_mgi_disable_monitor(void)
{
    uint32_t mon1_idx, mon2_idx;
    int status;

    /* Enable all monitor first */
    mgi_enable_all_monitor(&fake_smcf_mgi_reg);

    /* Clearing one monitor only */
    mon1_idx = 5;
    status = mgi_disable_monitor(&fake_smcf_mgi_reg, mon1_idx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_BIT_LOW(mon1_idx, fake_smcf_mgi_reg.MON_REQ);

    /* Clear second monitor doesn't affect the first value */
    mon2_idx = 9;

    status = mgi_disable_monitor(&fake_smcf_mgi_reg, mon2_idx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_BIT_LOW(mon1_idx, fake_smcf_mgi_reg.MON_REQ);
    TEST_ASSERT_BIT_LOW(mon2_idx, fake_smcf_mgi_reg.MON_REQ);
}

void utest_mgi_is_monitor_enabled(void)
{
    uint32_t mon1_idx, mon2_idx;
    bool enabled;

    mon1_idx = MGI_MON_IDX_MON9;
    mgi_enable_monitor(&fake_smcf_mgi_reg, mon1_idx);

    mon2_idx = MGI_MON_IDX_MON10;
    mgi_disable_monitor(&fake_smcf_mgi_reg, mon2_idx);

    /* Monitor 9 is enabled while monitor 10 is disabled */
    enabled = mgi_is_monitor_enabled(&fake_smcf_mgi_reg, MGI_MON_IDX_MON9);
    TEST_ASSERT_TRUE(enabled);

    enabled = mgi_is_monitor_enabled(&fake_smcf_mgi_reg, MGI_MON_IDX_MON10);
    TEST_ASSERT_FALSE(enabled);
}

void utest_mgi_get_number_of_mode_registers(void)
{
    const uint32_t expected_num_registers = 3;
    struct smcf_mgi_reg fake_mgi_reg = {
        .FEAT1 = ((expected_num_registers << SMCF_MGI_FEAT1_MODE_REG_POS) - 1),
    };
    uint32_t num_registers;

    num_registers = mgi_get_number_of_mode_registers(&fake_mgi_reg);

    TEST_ASSERT_EQUAL(expected_num_registers, num_registers);
}

void utest_mgi_get_number_of_bits_in_mode_registers(void)
{
    const uint32_t expected_num_mode_bits = 12;
    struct smcf_mgi_reg fake_mgi_reg = {
        .FEAT1 = ((expected_num_mode_bits << SMCF_MGI_FEAT1_MODE_LEN_POS) - 1),
    };
    uint32_t num_mode_bits;

    num_mode_bits = mgi_get_number_of_bits_in_mode_registers(&fake_mgi_reg);

    TEST_ASSERT_EQUAL(expected_num_mode_bits, num_mode_bits);
}

void utest_mgi_broadcast_mode_enable_and_disable_monitor(void)
{
    uint32_t mon1_idx, mon2_idx;
    int status;

    /* Enable program one monitor(7) and disable another(15) */
    mon1_idx = MGI_MON_IDX_MON7;
    mon2_idx = MGI_MON_IDX_MON15;

    status = mgi_enable_program_mode(&fake_smcf_mgi_reg, mon1_idx);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    status = mgi_enable_program_mode(&fake_smcf_mgi_reg, mon2_idx);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    TEST_ASSERT_BIT_HIGH(mon1_idx, fake_smcf_mgi_reg.MODE_BCAST);
    TEST_ASSERT_BIT_HIGH(mon2_idx, fake_smcf_mgi_reg.MODE_BCAST);
}

void utest_mgi_broadcast_mode_check_range(void)
{
    int status;

    status = mgi_enable_program_mode(&fake_smcf_mgi_reg, MGI_NUM_OF_MON);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_mgi_broadcast_mode_check_broadcast_mask(void)
{
    uint32_t mon_mask, mode_bcast_tmp;
    int status;

    /* Check broadcast mask */
    mode_bcast_tmp = fake_smcf_mgi_reg.MODE_BCAST;
    mon_mask = (1U << MGI_MON_IDX_MON2) & (1U << MGI_MON_IDX_MON5);

    status = mgi_enable_program_mode_multi(&fake_smcf_mgi_reg, mon_mask);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(mon_mask, (fake_smcf_mgi_reg.MODE_BCAST & mon_mask));
    TEST_ASSERT_EQUAL(
        mode_bcast_tmp, (fake_smcf_mgi_reg.MODE_BCAST & ~mon_mask));
}

void utest_mgi_is_monitor_mode_updated_false(void)
{
    uint32_t mode = MODE_VALUE;
    uint32_t mode_idx;
    uint32_t *ptr_set_const_reg;
    int status;
    bool updated;

    mode_idx = 0;
    status = mgi_set_monitor_mode(&fake_smcf_mgi_reg, mode_idx, mode);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    mode_idx = 1;
    status = mgi_set_monitor_mode(&fake_smcf_mgi_reg, mode_idx, mode);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    mode_idx = 2;
    status = mgi_set_monitor_mode(&fake_smcf_mgi_reg, mode_idx, mode);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /*
     * Only 3 REQ registers available. mode should written to REQ0,REQ1,REQ2
     * but not REQ3
     */
    TEST_ASSERT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ0);
    TEST_ASSERT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ1);
    TEST_ASSERT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ2);
    TEST_ASSERT_NOT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ3);

    /* Only STAT0 and STAT1 forced in fake_smcf */
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.MODE_STAT0;
    *ptr_set_const_reg = mode;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.MODE_STAT1;
    *ptr_set_const_reg = mode;

    updated = mgi_is_monitor_mode_updated(&fake_smcf_mgi_reg);
    TEST_ASSERT_FALSE(updated);
}

void utest_mgi_is_monitor_mode_updated_true(void)
{
    uint32_t mode = MODE_VALUE;
    uint32_t mode_idx;
    uint32_t *ptr_set_const_reg;
    int status;
    bool updated;

    mode_idx = 0;
    status = mgi_set_monitor_mode(&fake_smcf_mgi_reg, mode_idx, mode);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    mode_idx = 1;
    status = mgi_set_monitor_mode(&fake_smcf_mgi_reg, mode_idx, mode);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    mode_idx = 2;
    status = mgi_set_monitor_mode(&fake_smcf_mgi_reg, mode_idx, mode);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /*
     * Only 3 REQ registers available. mode should written to REQ0,REQ1,REQ2
     * but not REQ3
     */
    TEST_ASSERT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ0);
    TEST_ASSERT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ1);
    TEST_ASSERT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ2);
    TEST_ASSERT_NOT_EQUAL(mode, fake_smcf_mgi_reg.MODE_REQ3);

    /* Only STAT0 and STAT1 forced in fake_smcf */
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.MODE_STAT0;
    *ptr_set_const_reg = mode;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.MODE_STAT1;
    *ptr_set_const_reg = mode;

    /* Now force set STAT2 value */
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.MODE_STAT2;
    *ptr_set_const_reg = mode;

    updated = mgi_is_monitor_mode_updated(&fake_smcf_mgi_reg);
    TEST_ASSERT_TRUE(updated);
}

void utest_mgi_set_sample_type_periodic(void)
{
    uint32_t *ptr_set_const_reg;
    int status;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;
    *ptr_set_const_reg |= (1U << SMCF_MGI_FEAT0_PER_TIMER_POS);

    /* Set valid value */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_PERIODIC);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_TRUE(
        (fake_smcf_mgi_reg.SMP_CFG & SMCF_MGI_SMP_CFG_SMP_TYP) ==
        SMCF_MGI_SAMPLE_TYPE_PERIODIC);

    /* Set invalid value */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_COUNT);
    TEST_ASSERT_NOT_EQUAL(FWK_SUCCESS, status);

    /* Set unsupported type */
    status = mgi_set_sample_type(
        &fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_TRIGGER_INPUT);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_mgi_set_sample_type_trigger_input(void)
{
    uint32_t *ptr_set_const_reg;
    int status;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;
    *ptr_set_const_reg |= (1U << SMCF_MGI_FEAT0_TRIGGER_IN_POS);

    /* Set valid value */
    status = mgi_set_sample_type(
        &fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_TRIGGER_INPUT);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_TRUE(
        (fake_smcf_mgi_reg.SMP_CFG & SMCF_MGI_SMP_CFG_SMP_TYP) ==
        SMCF_MGI_SAMPLE_TYPE_TRIGGER_INPUT);

    /* Set invalid value */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_COUNT);
    TEST_ASSERT_NOT_EQUAL(FWK_SUCCESS, status);

    /* Set unsupported type */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_PERIODIC);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_mgi_set_sample_type_manual(void)
{
    int status;

    /* Set valid value */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_MANUAL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_TRUE(
        (fake_smcf_mgi_reg.SMP_CFG & SMCF_MGI_SMP_CFG_SMP_TYP) ==
        SMCF_MGI_SAMPLE_TYPE_MANUAL);

    /* Set invalid value */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_COUNT);
    TEST_ASSERT_NOT_EQUAL(FWK_SUCCESS, status);

    /* Set unsupported type */
    status =
        mgi_set_sample_type(&fake_smcf_mgi_reg, SMCF_MGI_SAMPLE_TYPE_PERIODIC);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_mgi_set_sample_period()
{
    int status;
    uint32_t *ptr_set_const_reg;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;
    *ptr_set_const_reg |= (1 << SMCF_MGI_FEAT0_PER_TIMER_POS);

    status = mgi_set_sample_period(&fake_smcf_mgi_reg, 32);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mgi_set_sample_period_not_supported()
{
    int status;
    uint32_t *ptr_set_const_reg;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT1;
    *ptr_set_const_reg &= ~(1 << SMCF_MGI_FEAT0_PER_TIMER_POS);

    status = mgi_set_sample_period(&fake_smcf_mgi_reg, 32);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_mgi_get_sample_delay_max()
{
    uint32_t max_delay;
    uint32_t *ptr_set_const_reg;

    /* Set MGI_SMP_DLY_LEN = 4 => max delay = 15 */
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT1;
    *ptr_set_const_reg |=
        (4 << SMCF_MGI_FEAT1_SMP_DLY_LEN_POS) & SMCF_MGI_FEAT1_SMP_DLY_LEN;

    max_delay = mgi_get_sample_delay_max(&fake_smcf_mgi_reg);
    TEST_ASSERT_EQUAL(max_delay, 15);
}

void utest_mgi_set_sample_delay()
{
    int status;
    uint32_t *ptr_set_const_reg;

    /* Set MGI_SMP_DLY_LEN = 4 bits => max delay = 15 */
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT1;
    *ptr_set_const_reg |=
        (4 << SMCF_MGI_FEAT1_SMP_DLY_LEN_POS) & SMCF_MGI_FEAT1_SMP_DLY_LEN;

    /* try setting delay as 8 */
    status = mgi_set_sample_delay(&fake_smcf_mgi_reg, 8);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* try setting delay as max 15 */
    status = mgi_set_sample_delay(&fake_smcf_mgi_reg, 15);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* try setting delay as more than configured max 16 */
    status = mgi_set_sample_delay(&fake_smcf_mgi_reg, 16);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_mgi_dma(void)
{
    uint32_t *ptr_set_const_reg;
    bool supported;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    *ptr_set_const_reg |= (1 << SMCF_MGI_FEAT0_DMA_IF_POS);

    supported = mgi_is_dma_supported(&fake_smcf_mgi_reg);
    TEST_ASSERT_TRUE(supported);
}

void utest_mgi_is_set_data_address_supported(void)
{
    uint32_t *ptr_set_const_reg;
    bool supported;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.DATA_INFO;

    *ptr_set_const_reg |= (1 << SMCF_DATA_INFO_ALT_ADDR_POS);

    supported = scmf_is_set_data_address_supported(&fake_smcf_mgi_reg);
    TEST_ASSERT_TRUE(supported);
}

void utest_mgi_is_set_data_address_supported_return_false(void)
{
    uint32_t *ptr_set_const_reg;
    bool supported;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.DATA_INFO;

    *ptr_set_const_reg = 0;

    supported = scmf_is_set_data_address_supported(&fake_smcf_mgi_reg);
    TEST_ASSERT_FALSE(supported);
}

void utest_mgi_is_data_packed(void)
{
    uint32_t *ptr_set_const_reg;
    bool packed;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.DATA_INFO;
    *ptr_set_const_reg |= (1 << SMCF_DATA_INFO_PACKED_POS);

    packed = mgi_is_data_packed(&fake_smcf_mgi_reg);
    TEST_ASSERT_TRUE(packed);
}

void utest_mgi_is_data_packed_return_false(void)
{
    uint32_t *ptr_set_const_reg;
    bool packed;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.DATA_INFO;
    *ptr_set_const_reg = 0;

    packed = mgi_is_data_packed(&fake_smcf_mgi_reg);
    TEST_ASSERT_FALSE(packed);
}

void utest_mgi_enable_dma(void)
{
    int status;
    uint32_t *ptr_set_const_reg;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    *ptr_set_const_reg = (1 << SMCF_MGI_FEAT0_DMA_IF_POS);

    status = mgi_enable_dma(&fake_smcf_mgi_reg);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mgi_enable_dma_not_supported(void)
{
    int status;
    uint32_t *ptr_set_const_reg;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    *ptr_set_const_reg = 0;
    status = mgi_enable_dma(&fake_smcf_mgi_reg);
    TEST_ASSERT_EQUAL(FWK_E_SUPPORT, status);
}

void utest_mgi_enable_dma_reg_value_correct(void)
{
    uint32_t *ptr_set_const_reg;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    *ptr_set_const_reg = (1 << SMCF_MGI_FEAT0_DMA_IF_POS);

    mgi_enable_dma(&fake_smcf_mgi_reg);

    TEST_ASSERT_BITS_HIGH(
        (1 << SMCF_MGI_WREN_DMA_IF_POS), fake_smcf_mgi_reg.WREN);
}

void utest_mgi_set_dma_address_not_supported(void)
{
    uint64_t address = 0xAA005500AA005500;
    uint32_t *ptr_set_const_reg;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    *ptr_set_const_reg = 0;

    TEST_ASSERT_EQUAL(
        FWK_E_SUPPORT, mgi_set_dma_data_address(&fake_smcf_mgi_reg, address));
}

void utest_mgi_set_dma_address_supported(void)
{
    uint64_t address = 0xAA005500AA005500;
    uint32_t *ptr_set_const_reg;
    int status;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    /* Set dma to supported */
    *ptr_set_const_reg = (1 << SMCF_MGI_FEAT0_DMA_IF_POS);

    status = mgi_set_dma_data_address(&fake_smcf_mgi_reg, address);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_mgi_set_dma_address_not_aligned(void)
{
    uint64_t not_32_bit_address = 0xAA005500AA005503;
    uint32_t *ptr_set_const_reg;
    int status;

    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    /* Set dma to supported */
    *ptr_set_const_reg = (1 << SMCF_MGI_FEAT0_DMA_IF_POS);

    status = mgi_set_dma_data_address(&fake_smcf_mgi_reg, not_32_bit_address);
    TEST_ASSERT_EQUAL(FWK_E_ALIGN, status);
}

void utest_mgi_set_dma_address_value(void)
{
    int status;
    uint64_t address = 0xAA005500AA005500;
    uint32_t *ptr_set_const_reg;
    ptr_set_const_reg = (uint32_t *)&fake_smcf_mgi_reg.FEAT0;

    /* Set dma to supported */
    *ptr_set_const_reg = (1 << SMCF_MGI_FEAT0_DMA_IF_POS);

    status = mgi_set_dma_data_address(&fake_smcf_mgi_reg, address);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_HEX64(
        address,
        (uint64_t)fake_smcf_mgi_reg.WADDR1 << 32 | fake_smcf_mgi_reg.WADDR0);
}

void utest_mgi_alternate_data_not_supported(void)
{
    bool alternate_supported = true;

    struct smcf_mgi_reg smcf_mgi = {
        .DATA_INFO = (0U << SMCF_DATA_INFO_ALT_ADDR_POS),
    };

    alternate_supported = mgi_is_alternate_data_address_supported(&smcf_mgi);

    TEST_ASSERT_FALSE(alternate_supported);
}

void utest_mgi_alternate_data_supported(void)
{
    bool alternate_supported = true;

    struct smcf_mgi_reg smcf_mgi = {
        .DATA_INFO = (1U << SMCF_DATA_INFO_ALT_ADDR_POS),
    };

    alternate_supported = mgi_is_alternate_data_address_supported(&smcf_mgi);

    TEST_ASSERT_TRUE(alternate_supported);
}

void utest_mgi_is_tag_supported_no(void)
{
    bool supported = true;
    struct smcf_mgi_reg mgi = {
        .FEAT0 = 0 << 25,
    };

    supported = mgi_is_tag_supported(&mgi);

    TEST_ASSERT_FALSE(supported);
}

void utest_mgi_is_tag_supported_yes(void)
{
    bool supported = false;
    struct smcf_mgi_reg mgi = {
        .FEAT0 = 1 << 25,
    };

    supported = mgi_is_tag_supported(&mgi);

    TEST_ASSERT_TRUE(supported);
}

void utest_mgi_get_tag_length(void)
{
    uint32_t const expectd_tag_length_in_bits = (3);
    struct smcf_mgi_reg mgi = {
        .FEAT0 = ((expectd_tag_length_in_bits - 1) << 8),
    };
    uint32_t tag_length_in_bits = 0;

    tag_length_in_bits = mgi_get_tag_length_in_bits(&mgi);

    TEST_ASSERT_EQUAL(expectd_tag_length_in_bits, tag_length_in_bits);
}

void utest_mgi_get_tag_length_max(void)
{
    uint32_t const expectd_tag_length_in_bits = (128);
    struct smcf_mgi_reg mgi = {
        .FEAT0 = ((expectd_tag_length_in_bits - 1) << 8),
    };
    uint32_t tag_length_in_bits = 0;

    tag_length_in_bits = mgi_get_tag_length_in_bits(&mgi);

    TEST_ASSERT_EQUAL(expectd_tag_length_in_bits, tag_length_in_bits);
}

void utest_mgi_get_tag_length_min(void)
{
    uint32_t const expectd_tag_length_in_bits = (1);
    struct smcf_mgi_reg mgi = {
        .FEAT0 = ((expectd_tag_length_in_bits - 1) << 8),
    };
    uint32_t tag_length_in_bits = 0;

    tag_length_in_bits = mgi_get_tag_length_in_bits(&mgi);

    TEST_ASSERT_EQUAL(expectd_tag_length_in_bits, tag_length_in_bits);
}

void utest_mgi_enable_tag_id_write_to_ram(void)
{
    struct smcf_mgi_reg mgi = {
        .WRCFG = 0,
    };

    mgi_enable_tag_id_write_to_ram(&mgi);

    TEST_ASSERT_BITS_HIGH(1 << 11, mgi.WRCFG);
}

void utest_mgi_enable_count_id_write_to_ram(void)
{
    struct smcf_mgi_reg mgi = {
        .WRCFG = 0,
    };

    mgi_enable_count_id_write_to_ram(&mgi);

    TEST_ASSERT_BITS_HIGH(1 << 10, mgi.WRCFG);
}

void utest_mgi_enable_valid_bits_write_to_ram(void)
{
    struct smcf_mgi_reg mgi = {
        .WRCFG = 0,
    };

    mgi_enable_valid_bits_write_to_ram(&mgi);

    TEST_ASSERT_BITS_HIGH(1 << 9, mgi.WRCFG);
}

void utest_mgi_enable_group_id_write_to_ram(void)
{
    struct smcf_mgi_reg mgi = {
        .WRCFG = 0,
    };

    mgi_enable_group_id_write_to_ram(&mgi);

    TEST_ASSERT_BITS_HIGH(1 << 8, mgi.WRCFG);
}

void utest_mgi_request_start_id_write_to_ram(void)
{
    struct smcf_mgi_reg mgi = {
        .WRCFG = 0,
    };

    mgi_request_start_id_wirte_to_ram(&mgi);

    TEST_ASSERT_BITS(0b11 << 4, 0b01 << 4, mgi.WRCFG);
}

void utest_mgi_request_start_and_end_id_write_to_ram(void)
{
    struct smcf_mgi_reg mgi = {
        .WRCFG = 0,
    };

    mgi_request_start_and_end_id_wirte_to_ram(&mgi);

    TEST_ASSERT_BITS(0b11 << 4, 0b10 << 4, mgi.WRCFG);
}

int mgi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(utest_mgi_get_num_of_monitors);
    RUN_TEST(utest_mgi_enable_monitor_one_monitor);
    RUN_TEST(utest_mgi_enable_monitor_boundary_case);
    RUN_TEST(utest_mgi_enable_monitor_out_of_range);
    RUN_TEST(utest_mgi_enable_all_monitor);
    RUN_TEST(utest_mgi_disable_monitor);
    RUN_TEST(utest_mgi_is_monitor_enabled);
    RUN_TEST(utest_mgi_get_number_of_mode_registers);
    RUN_TEST(utest_mgi_get_number_of_bits_in_mode_registers);
    RUN_TEST(utest_mgi_broadcast_mode_enable_and_disable_monitor);
    RUN_TEST(utest_mgi_broadcast_mode_check_range);
    RUN_TEST(utest_mgi_broadcast_mode_check_broadcast_mask);
    RUN_TEST(utest_mgi_is_monitor_mode_updated_false);
    RUN_TEST(utest_mgi_is_monitor_mode_updated_true);
    RUN_TEST(utest_mgi_set_sample_type_periodic);
    RUN_TEST(utest_mgi_set_sample_type_trigger_input);
    RUN_TEST(utest_mgi_set_sample_type_manual);
    RUN_TEST(utest_mgi_set_sample_period);
    RUN_TEST(utest_mgi_set_sample_period_not_supported);
    RUN_TEST(utest_mgi_get_sample_delay_max);
    RUN_TEST(utest_mgi_set_sample_delay);
    RUN_TEST(utest_mgi_dma);
    RUN_TEST(utest_mgi_is_set_data_address_supported);
    RUN_TEST(utest_mgi_is_set_data_address_supported_return_false);
    RUN_TEST(utest_mgi_is_data_packed);
    RUN_TEST(utest_mgi_is_data_packed_return_false);
    RUN_TEST(utest_mgi_enable_dma);
    RUN_TEST(utest_mgi_enable_dma_not_supported);
    RUN_TEST(utest_mgi_enable_dma_reg_value_correct);
    RUN_TEST(utest_mgi_set_dma_address_not_supported);
    RUN_TEST(utest_mgi_set_dma_address_supported);
    RUN_TEST(utest_mgi_set_dma_address_not_aligned);
    RUN_TEST(utest_mgi_set_dma_address_value);
    RUN_TEST(utest_mgi_alternate_data_not_supported);
    RUN_TEST(utest_mgi_alternate_data_supported);
    RUN_TEST(utest_mgi_is_tag_supported_no);
    RUN_TEST(utest_mgi_is_tag_supported_yes);
    RUN_TEST(utest_mgi_get_tag_length);
    RUN_TEST(utest_mgi_get_tag_length_max);
    RUN_TEST(utest_mgi_get_tag_length_min);
    RUN_TEST(utest_mgi_enable_tag_id_write_to_ram);
    RUN_TEST(utest_mgi_enable_count_id_write_to_ram);
    RUN_TEST(utest_mgi_enable_valid_bits_write_to_ram);
    RUN_TEST(utest_mgi_enable_group_id_write_to_ram);
    RUN_TEST(utest_mgi_request_start_id_write_to_ram);
    RUN_TEST(utest_mgi_request_start_and_end_id_write_to_ram);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return mgi_test_main();
}
#endif
