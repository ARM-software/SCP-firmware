/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP DDR-PHY driver
 */

#include "n1sdp_scp_mmap.h"

#include <n1sdp_ddr_phy_values.h>

#include <mod_n1sdp_ddr_phy.h>
#include <mod_n1sdp_dmc620.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define NUM_SLICES          9
#define NUM_BITS_PER_SLICE  8
#define NUM_DATA_PATTERNS   5
#define DCI_FIFO_SIZE       20

struct wrdq_eye {
    uint16_t min;
    uint8_t min_found;
    uint16_t max;
    uint8_t max_found;
    uint16_t mid;
    uint16_t width;
};

uint32_t wr_data_all[NUM_DATA_PATTERNS][DCI_FIFO_SIZE] = {
    {0x0, 0x0, 0x0, 0x0, 0x0000, 0x0, 0x0, 0x0, 0x0, 0x0000,
     0x0, 0x0, 0x0, 0x0, 0x0000, 0x0, 0x0, 0x0, 0x0, 0x0000,},
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF,
     0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF,
     0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF,
     0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF,},
    {0x0, 0x0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF00,
     0x0, 0x0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF00,
     0x0, 0x0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF00,
     0x0, 0x0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF00,},
    {0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x0, 0x00FF,
     0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x0, 0x00FF,
     0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x0, 0x00FF,
     0xFFFFFFFF, 0xFFFFFFFF, 0x0, 0x0, 0x00FF,},
    {0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A, 0x5AA5,
     0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A, 0x5AA5,
     0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A, 0x5AA5,
     0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A, 0x5AA5,}
};

struct slice_eye_stat {
  uint16_t min_width;
  int16_t median_mid;
};

struct wrdq_eye wrdq_eyes[NUM_SLICES][NUM_BITS_PER_SLICE];
struct wrdq_eye best_wrdq_eyes[NUM_SLICES][NUM_BITS_PER_SLICE];
uint16_t cur_wrdq_delays[NUM_SLICES][NUM_BITS_PER_SLICE];

struct slice_eye_stat slice_eye_stats[NUM_SLICES];
struct slice_eye_stat best_slice_eye_stats[NUM_SLICES];
int16_t sorted_mids[NUM_BITS_PER_SLICE];
uint32_t *wr_data;
uint32_t rd_data[DCI_FIFO_SIZE];
uint8_t wrrd_passes[NUM_SLICES][NUM_BITS_PER_SLICE];
uint16_t DEFAULT_DELAY = 0x240;

/*
 * Functions fulfilling this module's interface
 */
static int n1sdp_ddr_phy_config(fwk_id_t element_id, struct dimm_info *info)
{
    int element_idx;
    struct mod_n1sdp_ddr_phy_reg *ddr_phy;
    const struct mod_n1sdp_ddr_phy_element_config *element_config;

    fwk_assert(info != NULL);

    element_idx = fwk_id_get_element_idx(element_id);
    element_config = fwk_module_get_data(element_id);

    ddr_phy = (struct mod_n1sdp_ddr_phy_reg *)element_config->ddr;

    FWK_LOG_INFO(
        "[DDR-PHY] Initializing PHY at 0x%p for %" PRIu16 " MHz speed",
        (const void *)ddr_phy,
        info->speed);

    switch (info->speed) {
    case 1333:
        ddr_phy_config_1333(ddr_phy, info, element_idx);
        break;
    case 1200:
        ddr_phy_config_1200(ddr_phy, info, element_idx);
        break;
    case 800:
        ddr_phy_config_800(ddr_phy, info);
        break;
    default:
        FWK_LOG_INFO("[DDR-PHY] Unsupported frequency!");
        break;
    }

    return FWK_SUCCESS;
}

static void adjust_per_rank_rptr_update_value(uint32_t phy_addr,
    struct dimm_info *info)
{
    uint32_t i = 0;
    uint32_t value = 0;
    uint32_t orig_value = 0;
    uint32_t rank_1_rddqs_latency_adjust_value = 0;
    uint32_t rank_1_rddqs_gate_slave_delay_value = 0;
    uint32_t rank_1_x4_rddqs_latency_adjust_value = 0;
    uint32_t rank_1_x4_rddqs_gate_slave_delay_value = 0;
    uint32_t rank_2_rddqs_latency_adjust_value = 0;
    uint32_t rank_2_rddqs_gate_slave_delay_value = 0;
    uint32_t rank_2_x4_rddqs_latency_adjust_value = 0;
    uint32_t rank_2_x4_rddqs_gate_slave_delay_value = 0;

    for (i = 0; i < 9; i++) {
        orig_value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
        value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
        value = value & 0xFFFCFFFF;
        *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;
        value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
        rank_1_rddqs_latency_adjust_value = (value & 0x000F0000) >> 16;
        rank_1_rddqs_gate_slave_delay_value = (value & 0x000003FF);

        value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
        value = value | 0x00010000;
        *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;
        value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
        rank_2_rddqs_latency_adjust_value = (value & 0x000F0000) >> 16;
        rank_2_rddqs_gate_slave_delay_value = (value & 0x000003FF);

        if ((rank_1_rddqs_latency_adjust_value +
             ((rank_1_rddqs_gate_slave_delay_value & 0x00000100) >> 8)) !=
            (rank_2_rddqs_latency_adjust_value +
              ((rank_2_rddqs_gate_slave_delay_value & 0x0000100) >> 8))) {
            if ((rank_1_rddqs_gate_slave_delay_value & 0x000003FF) < 0x100) {
                value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
                value = (value & 0xFFFFFC00) | 0x104;
                *(uint32_t *)(phy_addr + (4 * (112 + (i * 256)))) = value;
            } else {
                value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
                value = (value & 0xFFFFFC00) | 0x0FC;
                *(uint32_t *)(phy_addr + (4 * (112 + (i * 256)))) = value;
            }
            value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
        }
        *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = orig_value;
    }

    if (info->dimm_mem_width == 4) {
        for (i = 0; i < 9; i++) {
            orig_value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
            value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
            value = value & 0xFFFCFFFF;
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;

            value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
            rank_1_x4_rddqs_latency_adjust_value = (value & 0x000F0000) >> 16;
            rank_1_x4_rddqs_gate_slave_delay_value = (value & 0x000003FF);

            value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
            value = value | 0x00010000;
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;

            value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
            rank_2_x4_rddqs_latency_adjust_value = (value & 0x000F0000) >> 16;
            rank_2_x4_rddqs_gate_slave_delay_value = (value & 0x000003FF);

            if ((rank_1_x4_rddqs_latency_adjust_value +
               ((rank_1_x4_rddqs_gate_slave_delay_value & 0x00000100) >> 8)) !=
                (rank_2_x4_rddqs_latency_adjust_value +
               ((rank_2_x4_rddqs_gate_slave_delay_value & 0x0000100) >> 8))) {
                if (rank_1_x4_rddqs_gate_slave_delay_value < 0x100) {
                    value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
                    value = (value & 0xFFFFFC00) | 0x104;
                    *(uint32_t *)(phy_addr + (4 * (116 + (i * 256)))) = value;
                } else {
                    value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
                    value = (value & 0xFFFFFC00) | 0x0FC;
                    *(uint32_t *)(phy_addr + (4 * (116 + (i * 256)))) = value;
                }
                value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
            }
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = orig_value;
        }
    }
}

static void delay_ms(uint32_t ms)
{
    volatile uint32_t i = 0;
    while (ms) {
        for (i = 0; i < 6000; i++)
            ;
        ms--;
    }
}

uint32_t dci_write_dram(struct mod_dmc620_reg *dmc, uint32_t *scp_address,
    uint32_t size_32, uint32_t rank, uint32_t bank)
{
    uint32_t count;
    uint8_t rank_1_hot = (1 << rank);

    // Confirm that DMC is in config state
    if ((dmc->MEMC_STATUS & 0x7) != 0x0) {
        FWK_LOG_INFO("DMC needs to be in config state");
        return FWK_E_STATE;
    }

    // Clear DCI write FIFO
    dmc->DIRECT_ADDR = 0x00000002;
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x000B;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_STATUS_MGR_ACTIVE) != 0)
        ;

    // Set DCI_STRB
    dmc->DCI_STRB = 0x0000000F;

    // Issue precharge direct command
    dmc->DIRECT_ADDR = 0x00000010;
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0002;
    dmc->DIRECT_ADDR = 0x00000000;

    // Fill the write buffer
    for (count = 0; count < size_32 ; count++)
        dmc->DCI_DATA = scp_address[count];

    // Set the column address to 0
    dmc->DIRECT_ADDR = 0x00000000;

    // Issue ACTIVATE command
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0009;

    // Issue WRITE command
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0008;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_STATUS_MGR_ACTIVE) != 0)
        ;

    // Issue precharge direct command
    dmc->DIRECT_ADDR = 0x00000010;
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0002;
    dmc->DIRECT_ADDR = 0x00000000;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_STATUS_MGR_ACTIVE) != 0)
        ;

    return FWK_SUCCESS;
}

uint32_t dci_read_dram(struct mod_dmc620_reg *dmc, uint32_t *scp_address,
    uint32_t size_32, uint32_t rank, uint32_t bank)
{
    uint32_t count = 0;
    uint8_t rank_1_hot = (1 << rank);

    // Confirm that DMC is in config state
    if ((dmc->MEMC_STATUS & 0x7) != 0x0) {
        FWK_LOG_INFO("DMC needs to be in config state");
        return FWK_E_PARAM;
    }

    // Clear DCI read FIFO
    dmc->DIRECT_ADDR = 0x00000004;
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x000B;

    // Issue ACTIVATE command
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0009;

    // Issue READ command
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0007;

    while ((dmc->MEMC_STATUS & MOD_DMC620_MEMC_STATUS_MGR_ACTIVE) != 0)
        ;

    // Copy read data back to buffer
    for (count = 0; count < size_32 ; count++)
        scp_address[count] = dmc->DCI_DATA;

    // Issue precharge direct command
    dmc->DIRECT_ADDR = 0x00000010;
    dmc->DIRECT_CMD = (rank_1_hot << 16) | 0x0002;
    dmc->DIRECT_ADDR = 0x00000000;

    return FWK_SUCCESS;
}


static int write_eye_detect_single_rank(fwk_id_t element_id,
    struct dimm_info *info, uint32_t rank, uint32_t delay_increment,
    uint32_t vrefdq_increment, uint32_t dbg_level)
{
    const uint32_t NUM_DQ_BITS = NUM_SLICES * NUM_BITS_PER_SLICE;
    const uint32_t NUM_DFI_BEATS_TO_CHECK = 4;
    const uint32_t SLICE_MASK = ~(0xFFFFFFFF << NUM_BITS_PER_SLICE);
    const uint32_t NUM_WORDS_IN_DFI_BEAT = 5;
    const uint8_t BIT_WRRD_SUCCESS =
        NUM_DATA_PATTERNS * NUM_DFI_BEATS_TO_CHECK * 2;
    const uint32_t PHY_PER_CS_TRAINING_INDEX_0_REG_IDX = 9;
    const uint32_t PHY_CLK_WRDQ0_SLAVE_DELAY_0_REG_IDX = 82;
    const uint32_t SC_PHY_MANUAL_UPDATE_REG_IDX = 2310;

    const uint16_t DELAY_MIN = 0x0;
    const uint16_t DELAY_MAX = 0x7FF;
    uint16_t DELAY_INCR = delay_increment;

    uint32_t orig_training_idx_vals[NUM_SLICES];

    bool sweep_vrefdq = (vrefdq_increment != 0);
    bool no_bits_pass;

    uint32_t ret_code = FWK_SUCCESS;
    uint32_t analysis_ret_code = FWK_SUCCESS;
    uint32_t denali_index = 0;
    uint32_t sc_phy_manual_update_reg_val = 0;
    uint32_t reg_val = 0;
    uint32_t wait_ms = 1;

    const int16_t MIN_VREFDQ_MR6 = 0x0;
    const int16_t MAX_VREFDQ_MR6 = 0x32;
    int16_t best_vrefdq_mr6 = -1;
    int32_t direction;
    uint32_t num_completed;
    uint32_t delay;
    uint32_t data_pattern;
    uint32_t i;
    uint32_t dfi_beat;
    uint32_t dfi_beat_word_offset;
    uint32_t dqs_edge;
    int16_t vrefdq_mr6;
    uint32_t slice;
    uint32_t bit;
    uint32_t rd_val;
    uint32_t wr_val;
    uint32_t eye_a_mid;
    uint32_t eye_b_mid;
    struct wrdq_eye *eye = NULL;
    struct wrdq_eye *eye_b = NULL;
    uint32_t wr_slice_data;
    uint32_t rd_slice_data;
    uint32_t start_bit;
    uint32_t word_num;
    uint32_t wr_bit;
    uint32_t rd_bit;
    uint16_t min_width;
    uint32_t num_good_eyes_in_slice;
    uint32_t better_slices;

    uint32_t speed;
    uint32_t range;
    uint32_t tccd_l;
    uint32_t direct_addr;
    uint32_t direct_cmd;
    int dmc_id;
    int s;
    int t;
    struct mod_dmc620_reg *dmc = NULL;
    uint32_t ddr_phy_base = 0;

    fwk_assert(rank < 2);
    dmc_id = fwk_id_get_element_idx(element_id);
    if (dmc_id == 0) {
        dmc = (struct mod_dmc620_reg *)SCP_DMC0;
        ddr_phy_base = SCP_DDR_PHY0;
    } else if (dmc_id == 1) {
        dmc = (struct mod_dmc620_reg *)SCP_DMC1;
        ddr_phy_base = SCP_DDR_PHY1;
    } else
        fwk_assert(false);

    best_vrefdq_mr6 = -1;
    for (slice = 0; slice < NUM_SLICES; slice++) {
        best_slice_eye_stats[slice].min_width = 0;
        best_slice_eye_stats[slice].median_mid = 0;
    }

    for (slice = 0; slice < NUM_SLICES; slice++) {
        for (bit = 0; bit < NUM_BITS_PER_SLICE; bit++) {
            best_wrdq_eyes[slice][bit].min = DELAY_MAX;
            best_wrdq_eyes[slice][bit].min_found = 0;
            best_wrdq_eyes[slice][bit].max = DELAY_MIN;
            best_wrdq_eyes[slice][bit].max_found = 0;
            best_wrdq_eyes[slice][bit].mid = 0;
            best_wrdq_eyes[slice][bit].width = 0;
        }
    }

    for (slice = 0; slice < NUM_SLICES; slice++) {
        rd_val = 0;
        wr_val = 0;
        denali_index = PHY_PER_CS_TRAINING_INDEX_0_REG_IDX + (slice * 256);
        rd_val = *(uint32_t *)(ddr_phy_base + (4 * denali_index));
        orig_training_idx_vals[slice] = rd_val;
        wr_val = (rank << 16) | (rd_val & 0xFFFFCFEFF);
        *(uint32_t *)(ddr_phy_base + (4 * denali_index)) = wr_val;
    }

    for (slice = 0; slice < NUM_SLICES; slice++) {
        for (bit = 0; bit < NUM_BITS_PER_SLICE; bit += 2) {
            rd_val = 0;
            denali_index = (PHY_CLK_WRDQ0_SLAVE_DELAY_0_REG_IDX + (bit / 2)) +
                            (slice * 256);
            rd_val = *(uint32_t *)(ddr_phy_base + (4 * denali_index));
        }
    }

    sc_phy_manual_update_reg_val = *(uint32_t *)(ddr_phy_base +
                                    (4 * SC_PHY_MANUAL_UPDATE_REG_IDX));

    speed = info->speed;
    range = 1;
    tccd_l =
        (speed == 800) ? 1 : (speed == 1200) ? 2 : 3;
    direct_addr = (tccd_l << 10) | (1 << 7) | (range - 1) << 6;
    direct_cmd = ((1 << rank) << 16) | (0x6 << 8) | 1;

    if (sweep_vrefdq) {
        dmc->DIRECT_ADDR = direct_addr;
        dmc->DIRECT_CMD = direct_cmd;
        delay_ms(wait_ms);
    }

    for (vrefdq_mr6 = MIN_VREFDQ_MR6;
         vrefdq_mr6 <= MAX_VREFDQ_MR6;
         vrefdq_mr6 += vrefdq_increment) {
        if (sweep_vrefdq) {
            direct_addr = (direct_addr & 0xFFFFFFC0) | vrefdq_mr6;
            dmc->DIRECT_ADDR = direct_addr;
            dmc->DIRECT_CMD = direct_cmd;
            delay_ms(wait_ms);
        }

        for (slice = 0; slice < NUM_SLICES; slice++) {
            for (bit = 0; bit < NUM_BITS_PER_SLICE; bit++) {
                wrdq_eyes[slice][bit].min = DELAY_MAX;
                wrdq_eyes[slice][bit].min_found = 0;
                wrdq_eyes[slice][bit].max = DELAY_MIN;
                wrdq_eyes[slice][bit].max_found = 0;
                wrdq_eyes[slice][bit].mid = 0;
                wrdq_eyes[slice][bit].width = 0;
            }
        }

        for (direction = -1; direction < 2; direction += 2) {
            num_completed = 0;
            delay = DEFAULT_DELAY;
            while (num_completed != NUM_DQ_BITS) {
                if ((delay < DELAY_MIN) || (delay > DELAY_MAX))
                    break;

                for (slice = 0; slice < NUM_SLICES; slice++) {
                    for (bit = 0; bit < NUM_BITS_PER_SLICE; bit++) {
                        cur_wrdq_delays[slice][bit] = delay;
                        if (bit % 2 == 1) {
                            reg_val =
                                (cur_wrdq_delays[slice][bit] & 0x7FF) << 16;
                            reg_val |=
                                (cur_wrdq_delays[slice][bit - 1] & 0x7FF);
                            denali_index =
                                (PHY_CLK_WRDQ0_SLAVE_DELAY_0_REG_IDX +
                                 (bit / 2)) + (slice*256);
                            *(uint32_t *)(ddr_phy_base + (4 * denali_index)) =
                                reg_val;
                        }
                    }
                }

                delay_ms(wait_ms);
                sc_phy_manual_update_reg_val |= 1;
                *(uint32_t *)(ddr_phy_base +
                              (4 * SC_PHY_MANUAL_UPDATE_REG_IDX)) =
                    sc_phy_manual_update_reg_val;

                memset(wrrd_passes, 0, sizeof(uint8_t) * NUM_DQ_BITS);

                for (data_pattern = 0;
                     data_pattern < NUM_DATA_PATTERNS;
                     data_pattern++) {
                    wr_data = wr_data_all[data_pattern];
                    ret_code = dci_write_dram(dmc, wr_data,
                                              DCI_FIFO_SIZE, rank, 0);
                    if (ret_code != FWK_SUCCESS)
                        return ret_code;
                    ret_code = dci_read_dram(dmc, rd_data,
                                             DCI_FIFO_SIZE, rank, 0);
                    if (ret_code != FWK_SUCCESS)
                        return ret_code;

                    for (dfi_beat = 0;
                         dfi_beat < NUM_DFI_BEATS_TO_CHECK;
                         dfi_beat++) {
                        dfi_beat_word_offset =
                            (dfi_beat * NUM_WORDS_IN_DFI_BEAT);
                        for (dqs_edge = 0; dqs_edge < 2; dqs_edge++) {
                            no_bits_pass = true;
                            for (slice = 0; slice < NUM_SLICES; slice++) {
                                start_bit = (dqs_edge * 64) +
                                             (slice * NUM_BITS_PER_SLICE);

                                if (slice == NUM_SLICES - 1)
                                    start_bit += (dqs_edge % 2 == 0) ? 64 : 8;

                                word_num = (start_bit / 32) +
                                            dfi_beat_word_offset;
                                wr_slice_data =
                                    (wr_data[word_num] >> (start_bit % 32)) &
                                    SLICE_MASK;
                                rd_slice_data =
                                    (rd_data[word_num] >> (start_bit % 32)) &
                                    SLICE_MASK;

                                for (bit = 0;
                                     bit < NUM_BITS_PER_SLICE; bit++) {
                                    wr_bit = (wr_slice_data >> bit) & 0x1;
                                    rd_bit = (rd_slice_data >> bit) & 0x1;
                                    if (wr_bit == rd_bit) {
                                        wrrd_passes[slice][bit]++;
                                        no_bits_pass = false;
                                    }
                                }
                            }

                            if (dbg_level == 0 && no_bits_pass) {
                                dqs_edge = 2;
                                dfi_beat = NUM_DFI_BEATS_TO_CHECK;
                                data_pattern = NUM_DATA_PATTERNS;
                                break;
                            }
                        }
                    }
                }

                for (slice = 0; slice < NUM_SLICES; slice++) {
                    for (bit = 0; bit < NUM_BITS_PER_SLICE; bit++) {
                        if (((direction < 0) &&
                             (wrdq_eyes[slice][bit].min_found == 1)) ||
                            ((direction > 0) &&
                             (wrdq_eyes[slice][bit].max_found == 1))) {
                            continue;
                        }

                        if (wrrd_passes[slice][bit] == BIT_WRRD_SUCCESS) {
                            if (direction < 0) {
                                if (cur_wrdq_delays[slice][bit] <
                                    wrdq_eyes[slice][bit].min) {
                                    wrdq_eyes[slice][bit].min =
                                        cur_wrdq_delays[slice][bit];
                                }
                            } else if (direction > 0) {
                                if (cur_wrdq_delays[slice][bit] >
                                    wrdq_eyes[slice][bit].max) {
                                    wrdq_eyes[slice][bit].max =
                                        cur_wrdq_delays[slice][bit];
                                }
                            }
                        } else {
                            if ((direction < 0) &&
                                (wrdq_eyes[slice][bit].min != DELAY_MAX)) {
                                wrdq_eyes[slice][bit].min_found = 1;
                                num_completed++;
                            }
                            if ((direction > 0) &&
                                (wrdq_eyes[slice][bit].max != DELAY_MIN)) {
                                wrdq_eyes[slice][bit].max_found = 1;
                                num_completed++;
                            }
                        }
                    }
                }
                delay += (direction*DELAY_INCR);
            }
        }

        for (slice = 0; slice < NUM_SLICES; slice++) {
            min_width = DELAY_MAX;
            num_good_eyes_in_slice = 0;
            for (i = 0; i < NUM_BITS_PER_SLICE; i++)
                sorted_mids[i] = DELAY_MAX;
            for (bit = 0; bit < NUM_BITS_PER_SLICE; bit++) {
                eye = &wrdq_eyes[slice][bit];
                if (!eye->min_found && !eye->max_found)
                    break;
                eye->width = eye->max - eye->min;
                if (eye->width < min_width)
                    min_width = eye->width;

                eye->mid = (eye->min + eye->max) / 2;
                for (s = 0; s < (int)NUM_BITS_PER_SLICE; s++) {
                    if (eye->mid < sorted_mids[s]) {
                        for (t = num_good_eyes_in_slice-1; t >= 0; t--)
                            sorted_mids[t+1] = sorted_mids[t];
                        sorted_mids[s] = eye->mid;
                        break;
                    }
                }
                num_good_eyes_in_slice++;
            }
            slice_eye_stats[slice].min_width =
                (min_width == DELAY_MAX) ? 0 : min_width;
            slice_eye_stats[slice].median_mid =
                sorted_mids[(num_good_eyes_in_slice+1)/2];
        }

        better_slices = 0;
        for (slice = 0; slice < NUM_SLICES; slice++) {
            if (slice_eye_stats[slice].min_width >
                best_slice_eye_stats[slice].min_width)
                better_slices++;
        }
        if (better_slices == NUM_SLICES) {
            best_vrefdq_mr6 = vrefdq_mr6;
            memcpy(best_wrdq_eyes, wrdq_eyes,
                sizeof(struct wrdq_eye)*NUM_SLICES*NUM_BITS_PER_SLICE);
            memcpy(best_slice_eye_stats, slice_eye_stats,
                sizeof(struct slice_eye_stat)*NUM_SLICES);
        }
        if (!sweep_vrefdq)
            break;
    }

    if (best_vrefdq_mr6 != -1) {
        for (slice = 0; slice < NUM_SLICES; slice++) {
            for (bit = 0; bit < NUM_BITS_PER_SLICE; bit++) {
                eye = &best_wrdq_eyes[slice][bit];
                if (eye->max == 0)
                    analysis_ret_code = FWK_E_RANGE;
            }
        }
    }

    if (best_vrefdq_mr6 == -1) {
        analysis_ret_code = FWK_E_RANGE;
        direct_addr &= 0xFFFFFF7F;
        dmc->DIRECT_ADDR = direct_addr;
        dmc->DIRECT_CMD = direct_cmd;
        delay_ms(wait_ms);
    } else {
        if (sweep_vrefdq) {
            direct_addr = (direct_addr & 0xFFFFFFC0) | best_vrefdq_mr6;
            dmc->DIRECT_ADDR = direct_addr;
            dmc->DIRECT_CMD = direct_cmd;
            delay_ms(wait_ms);
            direct_addr &= 0xFFFFFF7F;
            dmc->DIRECT_ADDR = direct_addr;
            dmc->DIRECT_CMD = direct_cmd;
            delay_ms(wait_ms);
        }

        for (slice = 0; slice < NUM_SLICES; slice++) {
            for (bit = 0; bit < NUM_BITS_PER_SLICE; bit += 2) {
                eye = &best_wrdq_eyes[slice][bit];
                eye_a_mid = eye->mid;
                eye_b = &best_wrdq_eyes[slice][bit + 1];
                eye_b_mid = eye_b->mid;
                reg_val = (eye_b_mid << 16) | eye_a_mid;
                denali_index = (PHY_CLK_WRDQ0_SLAVE_DELAY_0_REG_IDX +
                                (bit / 2)) + (slice * 256);
                *(uint32_t *)(ddr_phy_base + (4 * denali_index)) = reg_val;
            }
        }

        delay_ms(wait_ms);
        sc_phy_manual_update_reg_val |= 1;
        *(uint32_t *)(ddr_phy_base + (4 * SC_PHY_MANUAL_UPDATE_REG_IDX)) =
            sc_phy_manual_update_reg_val;
    }

    for (slice = 0; slice < NUM_SLICES; slice++) {
        wr_val = 0;
        denali_index = PHY_PER_CS_TRAINING_INDEX_0_REG_IDX + (slice * 256);
        wr_val = orig_training_idx_vals[slice];
        *(uint32_t *)(ddr_phy_base + (4 * denali_index)) = wr_val;
    }

    if (ret_code != FWK_SUCCESS)
      return ret_code;

    return analysis_ret_code;
}

static int write_eye_detect(fwk_id_t element_id, struct dimm_info *info,
    uint32_t rank_sel, uint32_t delay_increment,
    uint32_t vrefdq_increment, uint32_t dbg_level)
{
    uint32_t start_rank;
    uint32_t stop_rank;
    uint32_t rank;
    int status = FWK_SUCCESS;

    if (((int)rank_sel > (info->number_of_ranks - 1)) && (rank_sel != 0xF)) {
        FWK_LOG_INFO("[DDR-PHY] Invalid rank parameter %" PRIu32, rank_sel);
        return FWK_E_PARAM;
    }

    if (rank_sel == 0xF) {
        start_rank = 0;
        stop_rank = info->number_of_ranks - 1;
    } else {
        start_rank = rank_sel;
        stop_rank = rank_sel;
    }

    for (rank = start_rank; rank <= stop_rank; rank++) {
        status = write_eye_detect_single_rank(element_id, info, rank,
            delay_increment, vrefdq_increment, dbg_level);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO(
                "[DDR-PHY] WET single rank failed with error %d", status);
            break;
        }
    }

    return status;
}

static int n1sdp_ddr_phy_post_training_configure(fwk_id_t element_id,
    struct dimm_info *info)
{
    int status;
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t h;
    uint32_t phy_addr;
    uint32_t value;
    uint32_t temp;
    uint32_t rddqs_latency_adjust_value;
    uint32_t rddqs_gate_slave_delay_value;
    uint32_t rddqs_x4_latency_adjust_value;
    uint32_t rddqs_x4_gate_slave_delay_value;
    uint32_t rptr_update_value;
    uint32_t rddata_valid_value;

    fwk_assert(info != NULL);

    element_config = fwk_module_get_data(element_id);
    phy_addr = (uint32_t)element_config->ddr;
    rddata_valid_value = 0;

    FWK_LOG_INFO("[DDR-PHY] Post training PHY setting at 0x%" PRIX32, phy_addr);

    for (i = 0; i < 9; i++)  {
        value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
        rddqs_latency_adjust_value = (value & 0x000F0000) >> 16;
        rddqs_gate_slave_delay_value = (value & 0x000003FF);
        value = *(uint32_t *)(phy_addr + (4 * (129 + (i * 256))));
        rptr_update_value = (value & 0x00000F00) >> 8;
        value = rddqs_latency_adjust_value +
                ((rddqs_gate_slave_delay_value & 0x00000100) >> 8) +
                rptr_update_value;
        if (value > rddata_valid_value)
            rddata_valid_value = value;
    }
    if (info->dimm_mem_width == 4) {
        for (i = 0; i < 9; i++) {
            value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
            rddqs_x4_latency_adjust_value = (value & 0x000F0000) >> 16;
            rddqs_x4_gate_slave_delay_value = (value & 0x000003FF);
            value = *(uint32_t *)(phy_addr + (4 * (133 + (i * 256))));
            rptr_update_value = (value & 0x000F0000) >> 16;
            value = rddqs_x4_latency_adjust_value +
                    ((rddqs_x4_gate_slave_delay_value & 0x00000100) >> 8) +
                    rptr_update_value;
            if (value > rddata_valid_value)
                rddata_valid_value = value;
        }
    }

    for (i = 0; i < 9; i++)  {
        value = *(uint32_t *)(phy_addr + (4 * (112 + (i * 256))));
        rddqs_latency_adjust_value = (value & 0x000F0000) >> 16;
        rddqs_gate_slave_delay_value = (value & 0x000003FF);
        value = rddqs_latency_adjust_value +
                ((rddqs_gate_slave_delay_value & 0x00000100) >> 8);
        rptr_update_value = rddata_valid_value - value;
        value = *(uint32_t *)(phy_addr + (4 * (129 + (i * 256))));
        if (rptr_update_value != ((value & 0x00000F00) >> 8)) {
            value = (value & 0xFFFFF0FF) | (rptr_update_value << 8);
            *(uint32_t *)(phy_addr + (4 * (129 + (i * 256)))) = value;
            value = *(uint32_t *)(phy_addr + (4 * (129 + (i * 256))));
        }
    }
    if (info->dimm_mem_width == 4) {
        for (i = 0; i < 9; i++) {
            value = *(uint32_t *)(phy_addr + (4 * (116 + (i * 256))));
            rddqs_x4_latency_adjust_value = (value & 0x000F0000) >> 16;
            rddqs_x4_gate_slave_delay_value = (value & 0x000003FF);
            value = rddqs_x4_latency_adjust_value +
               ((rddqs_x4_gate_slave_delay_value & 0x00000100) >> 8);
            rptr_update_value = rddata_valid_value - value;
            value = *(uint32_t *)(phy_addr + (4 * (133 + (i * 256))));
            if (rptr_update_value != ((value & 0x000F0000) >> 16)) {
                value = (value & 0xFFF0FFFF) | (rptr_update_value << 16);
                *(uint32_t *)(phy_addr + (4 * (133 + (i * 256)))) = value;
                value = *(uint32_t *)(phy_addr + (4 * (133 + (i * 256))));
            }
        }
    }

    if (info->number_of_ranks > 1)
        adjust_per_rank_rptr_update_value(phy_addr, info);

    if (info->speed >= 1333) {
        FWK_LOG_INFO("[DDR-PHY] Performing write eye training...");
        status = write_eye_detect(element_id, info, 0xF, 0x4, 0x2, 0);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[DDR-PHY] FAIL!");
            return status;
        }
        FWK_LOG_INFO("[DDR-PHY] PASS!");
    }

    for (h = 0; h < info->number_of_ranks; h++) {
        for (i = 0; i < 9; i++) {
            value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
            temp = value;
            value = (value & 0xFFFCFFFF) | (h << 16);
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;
            value = *(uint32_t *)(phy_addr + (4 * (17 + (i * 256))));
            value = (value & 0xFFFF00FF) | 0x100;
            *(uint32_t *)(phy_addr + (4 * (17 + (i * 256)))) = value;
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = temp;
        }
    }

    return FWK_SUCCESS;
}

static int n1sdp_verify_phy_status(fwk_id_t element_id,
    uint8_t training_type, struct dimm_info *info)
{
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t j;
    uint32_t h;
    uint32_t phy_base;
    uint32_t value1;
    uint32_t value2;
    uint32_t num_slices;

    fwk_assert(info != NULL);

    element_config = fwk_module_get_data(element_id);
    phy_base = (uint32_t)element_config->ddr;
    num_slices = 18;

    for (h = 0; h < info->number_of_ranks; h++) {
        switch (training_type) {
        case DDR_ADDR_TRAIN_TYPE_WR_LVL:
            for (i = 0; i < 9; i++) {
                value1 = *(uint32_t *)(phy_base + (4 * (9 + (i * 256))));
                value1 = (value1 & 0xFFFCFFFF) | (h << 16);
                *(uint32_t *)(phy_base + (4 * (9 + (i * 256)))) = value1;
                value1 = *(uint32_t *)(phy_base + (4 * (17 + (i * 256))));
                value1 = (value1 & 0xFFFF00FF) | 0x100;
                *(uint32_t *)(phy_base + (4 * (17 + (i * 256)))) = value1;
                value1 = *(uint32_t *)(phy_base + (4 * (41 + (i * 256))));
                value2 = *(uint32_t *)(phy_base + (4 * (42 + (i * 256))));
                if (((value1 >> 16) >= 0x0200) ||
                    ((value2 & 0x0000FFFF) >= 0x200)) {
                    FWK_LOG_INFO(
                        "[DDR-PHY] PHY 0x%" PRIX32
                        " : Invalid Hard0/Hard 1 "
                        "value found for slice %" PRIu32,
                        phy_base,
                        i);
                }
            }
            break;
        case DDR_ADDR_TRAIN_TYPE_RD_GATE:
            for (i = 0; i < 9; i++) {
                value1 = *(uint32_t *)(phy_base + (4 * (9 + (i * 256))));
                value1 = (value1 & 0xFFFCFFFF) | (h << 16);
                *(uint32_t *)(phy_base + (4 * (9 + (i * 256)))) = value1;
                value1 = *(uint32_t *)(phy_base + (4 * (17 + (i * 256))));
                value1 = (value1 & 0xFFFF00FF) | 0x100;
                *(uint32_t *)(phy_base + (4 * (17 + (i * 256)))) = value1;
                value1 = *(uint32_t *)(phy_base + (4 * (46 + (i * 256))));
                if ((value1 != 0x003C) &&
                    ((info->dimm_mem_width == 4) && (value1 != 0x13C))) {
                    FWK_LOG_INFO(
                        "[DDR-PHY] PHY 0x%" PRIX32
                        " : Final read gate training"
                        " status != 0x003C for slice %" PRIu32,
                        phy_base,
                        i);
                }
            }
            break;
        case DDR_ADDR_TRAIN_TYPE_RD_EYE:
            for (i = 0; i < 9; i++) {
                value1 = *(uint32_t *)(phy_base + (4 * (9 + (i * 256))));
                value1 = (value1 & 0xFFFCFFFF) | (h << 16);
                *(uint32_t *)(phy_base + (4 * (9 + (i * 256)))) = value1;
                value1 = *(uint32_t *)(phy_base + (4 * (17 + (i * 256))));
                value1 = (value1 & 0xFFFF00FF) | 0x100;
                *(uint32_t *)(phy_base + (4 * (17 + (i * 256)))) = value1;
                for (j = 0; j < num_slices; j++) {
                    value1 = *(uint32_t *)(phy_base + (4 * (34 + (i * 256))));
                    if (j <= 16)
                        value1 = (value1 & 0xFF00FFFF) | (j << 16);
                    else
                        value1 = (value1 & 0xFF00FFFF) | (0x18 << 16);

                    *(uint32_t *)(phy_base + (4 * (34 + (i * 256)))) = value1;
                    value1 = *(uint32_t *)(phy_base + (4 * (47 + (i * 256))));
                    if ((value1 & 0x0000FFFF) > 0x0180) {
                        FWK_LOG_INFO(
                            "[DDR-PHY] PHY 0x%" PRIX32 " : slice %" PRIu32
                            " phy_rdlvl_rddqs_dq_le_dly_obs_%" PRIu32
                            " is > 0x180",
                            phy_base,
                            j,
                            i);
                    }
                    if ((value1 >> 16) > 0x0180) {
                        FWK_LOG_INFO(
                            "[DDR-PHY] PHY 0x%" PRIX32 " : slice %" PRIu32
                            " phy_rdlvl_rddqs_dq_te_dly_obs_%" PRIu32
                            " is > 0x180",
                            phy_base,
                            j,
                            i);
                    }
                    value1 = *(uint32_t *)(phy_base + (4 * (49 + (i * 256))));
                    if ((value1 >> 16) != 0x0C00) {
                        FWK_LOG_INFO(
                            "[DDR-PHY] PHY 0x%" PRIX32
                            " : Final read data eye "
                            "training "
                            "status != 0x0C00 for slice %" PRIu32,
                            phy_base,
                            i);
                    }
                }
            }
            break;
        case DDR_ADDR_TRAIN_TYPE_VREF:
            for (i = 0; i < 9; i++) {
                value1 = *(uint32_t *)(phy_base + (4 * (9 + (i * 256))));
                value1 = (value1 & 0xFFFCFFFF) | (h << 16);
                *(uint32_t *)(phy_base + (4 * (9 + (i * 256)))) = value1;
                value1 = *(uint32_t *)(phy_base + (4 * (17 + (i * 256))));
                value1 = (value1 & 0xFFFF00FF) | 0x100;
                *(uint32_t *)(phy_base + (4 * (17 + (i * 256)))) = value1;
            }
            break;
        default:
            return FWK_E_STATE;
            break;
        }
    }
    return FWK_SUCCESS;
}

static int n1sdp_wrlvl_phy_obs_regs(fwk_id_t element_id,
    uint32_t rank, struct dimm_info *info)
{
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t j;
    uint32_t h;
    uint32_t phy_addr;
    uint32_t value;
    uint32_t rank_start;
    uint32_t rank_end;

    fwk_assert(info != NULL);

    element_config = fwk_module_get_data(element_id);
    phy_addr = (uint32_t)element_config->ddr;

    if (rank == 0) {
        rank_start = 0;
        rank_end = info->number_of_ranks;
    } else {
        rank_start = rank - 1;
        rank_end = rank;
    }

    for (h = rank_start; h < rank_end; h++) {
        for (i = 0; i < 9; i++) {
            value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
            value = (value & 0xFFFCFFFF) | (h << 16);
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;
            value = *(uint32_t *)(phy_addr + (4 * (17 + (i * 256))));
            value = (value & 0xFFFF00FF) | 0x100;
            *(uint32_t *)(phy_addr + (4 * (17 + (i * 256)))) = value;

            for (j = 0; j < 10; j++) {
                value = *(uint32_t *)(phy_addr + (4 * (31 + (i * 256))));
                value = (value & 0xFFFF00FF) | (j << 8);
                *(uint32_t *)(phy_addr + (4 * (31 + (i * 256)))) = value;
            }
            for (j = 0; j < 10; j++) {
                value = *(uint32_t *)(phy_addr + (4 * (31 + (i * 256))));
                value = (value & 0xFFF0FFFF) | (j << 16);
                *(uint32_t *)(phy_addr + (4 * (31 + (i * 256)))) = value;
            }
        }
    }
    return FWK_SUCCESS;
}

static int n1sdp_read_gate_phy_obs_regs(fwk_id_t element_id,
    uint32_t rank, struct dimm_info *info)
{
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t h;
    uint32_t phy_addr;
    uint32_t value;
    uint32_t rank_start;
    uint32_t rank_end;

    fwk_assert(info != NULL);

    element_config = fwk_module_get_data(element_id);
    phy_addr = (uint32_t)element_config->ddr;

    if (rank == 0) {
        rank_start = 0;
        rank_end = info->number_of_ranks;
    } else {
        rank_start = rank - 1;
        rank_end = rank;
    }

    for (h = rank_start; h < rank_end; h++) {
        for (i = 0; i < 9; i++) {
          value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
          value = (value & 0xFFFCFFFF) | (h << 16);
          *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;
          value = *(uint32_t *)(phy_addr + (4 * (17 + (i * 256))));
          value = (value & 0xFFFF00FF) | 0x100;
          *(uint32_t *)(phy_addr + (4 * (17 + (i * 256)))) = value;
        }
    }
    return FWK_SUCCESS;
}

static int n1sdp_read_eye_phy_obs_regs(fwk_id_t element_id,
    struct dimm_info *info)
{
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t j;
    uint32_t h;
    uint32_t phy_addr;
    uint32_t value;
    uint32_t num_dq_slices = 18;

    fwk_assert(info != NULL);

    element_config = fwk_module_get_data(element_id);
    phy_addr = (uint32_t)element_config->ddr;

    for (h = 0; h < info->number_of_ranks; h++) {
        for (i = 0; i < 9; i++) {
            value = *(uint32_t *)(phy_addr + (4 * (9 + (i * 256))));
            value = (value & 0xFFFCFFFF) | (h << 16);
            *(uint32_t *)(phy_addr + (4 * (9 + (i * 256)))) = value;
            value = *(uint32_t *)(phy_addr + (4 * (17 + (i * 256))));
            value = (value & 0xFFFF00FF) | 0x100;
            *(uint32_t *)(phy_addr + (4 * (17 + (i * 256)))) = value;
            for (j = 0; j < num_dq_slices; j++) {
                value = *(uint32_t *)(phy_addr + (4 * (34 + (i * 256))));
                if (j <= 16)
                    value = (value & 0xFF00FFFF) | (j << 16);
                else
                    value = (value & 0xFF00FFFF) | (0x18 << 16);

                *(uint32_t *)(phy_addr + (4 * (34 + (i * 256)))) = value;
            }
        }
    }
    return FWK_SUCCESS;
}

static int n1sdp_phy_obs_regs(fwk_id_t element_id,
    uint32_t rank, struct dimm_info *info)
{
    int status;

    status = n1sdp_wrlvl_phy_obs_regs(element_id, rank, info);
    if (status != FWK_SUCCESS)
        return status;

    status = n1sdp_read_gate_phy_obs_regs(element_id, rank, info);
    if (status != FWK_SUCCESS)
        return status;

    return n1sdp_read_eye_phy_obs_regs(element_id, info);
}

static struct mod_dmc_ddr_phy_api n1sdp_ddr_phy_api = {
    .configure = n1sdp_ddr_phy_config,
    .post_training_configure = n1sdp_ddr_phy_post_training_configure,
    .verify_phy_status = n1sdp_verify_phy_status,
    .wrlvl_phy_obs_regs = n1sdp_wrlvl_phy_obs_regs,
    .read_gate_phy_obs_regs = n1sdp_read_gate_phy_obs_regs,
    .phy_obs_regs = n1sdp_phy_obs_regs,
};

/*
 * Functions fulfilling the framework's module interface
 */

static int n1sdp_ddr_phy_init(fwk_id_t module_id, unsigned int element_count,
    const void *config)
{
    return FWK_SUCCESS;
}

static int n1sdp_ddr_phy_element_init(fwk_id_t element_id, unsigned int unused,
    const void *data)
{
    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int n1sdp_ddr_phy_bind(fwk_id_t id, unsigned int round)
{
    /* Skip the second round (rounds are zero-indexed) */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to be done for element-level binding */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    return FWK_SUCCESS;
}

static int n1sdp_ddr_phy_process_bind_request(fwk_id_t requester_id,
    fwk_id_t id, fwk_id_t api_type, const void **api)
{
    /* Binding to elements is not permitted. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_E_ACCESS;

    *api = &n1sdp_ddr_phy_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_ddr_phy = {
    .name = "N1SDP DDR-PHY",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = n1sdp_ddr_phy_init,
    .element_init = n1sdp_ddr_phy_element_init,
    .bind = n1sdp_ddr_phy_bind,
    .process_bind_request = n1sdp_ddr_phy_process_bind_request,
    .api_count = 1,
};
