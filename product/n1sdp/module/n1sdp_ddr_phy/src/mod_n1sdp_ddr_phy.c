/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP DDR-PHY driver
 */

#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <internal/n1sdp_ddr_phy.h>
#include <mod_n1sdp_ddr_phy.h>
#include <mod_n1sdp_dmc620.h>
#include <mod_log.h>
#include <n1sdp_ddr_phy_values.h>

static struct mod_log_api *log_api;

/*
 * Functions fulfilling this module's interface
 */
static int n1sdp_ddr_phy_config(fwk_id_t element_id, struct dimm_info *info)
{
    int status;
    int element_idx;
    struct mod_n1sdp_ddr_phy_reg *ddr_phy;
    const struct mod_n1sdp_ddr_phy_element_config *element_config;

    fwk_assert(info != NULL);

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS)
        return status;

    element_idx = fwk_id_get_element_idx(element_id);
    element_config = fwk_module_get_data(element_id);

    ddr_phy = (struct mod_n1sdp_ddr_phy_reg *)element_config->ddr;

    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR-PHY] Initializing PHY at 0x%x for %d MHz speed\n",
        (uintptr_t)ddr_phy, info->speed);

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
        log_api->log(MOD_LOG_GROUP_INFO, "[DDR-PHY] Unsupported frequency!\n");
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

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS)
        return status;

    element_config = fwk_module_get_data(element_id);
    phy_addr = (uint32_t)element_config->ddr;
    rddata_valid_value = 0;

    log_api->log(MOD_LOG_GROUP_INFO,
        "[DDR-PHY] Post training PHY setting at 0x%x\n", phy_addr);

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
    int status;
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t j;
    uint32_t h;
    uint32_t phy_base;
    uint32_t value1;
    uint32_t value2;
    uint32_t num_slices;

    fwk_assert(info != NULL);

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS)
        return status;

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
                log_api->log(MOD_LOG_GROUP_INFO,
                    "[DDR-PHY] PHY 0x%08x : Invalid Hard0/Hard 1 value found "
                    "for slice %d\n", phy_base, i);
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
                    log_api->log(MOD_LOG_GROUP_INFO,
                        "[DDR-PHY] PHY 0x%08x : Final read gate training "
                        "status != 0x003C for slice %d\n", phy_base, i);
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
                        log_api->log(MOD_LOG_GROUP_INFO,
                            "[DDR-PHY] PHY 0x%08x : slice %d "
                            " phy_rdlvl_rddqs_dq_le_dly_obs_%d is > 0x180\n",
                            phy_base, j, i);
                    }
                    if ((value1 >> 16) > 0x0180) {
                        log_api->log(MOD_LOG_GROUP_INFO,
                        "[DDR-PHY] PHY 0x%08x : slice %d "
                        "phy_rdlvl_rddqs_dq_te_dly_obs_%d is > 0x180\n",
                        phy_base, j, i);
                    }
                    value1 = *(uint32_t *)(phy_base + (4 * (49 + (i * 256))));
                    if ((value1 >> 16) != 0x0C00) {
                        log_api->log(MOD_LOG_GROUP_INFO,
                            "[DDR-PHY] PHY 0x%08x : Final read data eye training "
                            "status != 0x0C00 for slice %d\n", phy_base, i);
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
    int status;
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t j;
    uint32_t h;
    uint32_t phy_addr;
    uint32_t value;
    uint32_t rank_start;
    uint32_t rank_end;

    fwk_assert(info != NULL);

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS)
        return status;

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
    int status;
    const struct mod_n1sdp_ddr_phy_element_config *element_config;
    uint32_t i;
    uint32_t h;
    uint32_t phy_addr;
    uint32_t value;
    uint32_t rank_start;
    uint32_t rank_end;

    fwk_assert(info != NULL);

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS)
        return status;

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
    int status;

    /* Skip the second round (rounds are zero-indexed) */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to be done for element-level binding */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    /* Bind to the log module and get a pointer to its API */
    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG), MOD_LOG_API_ID,
        &log_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

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
