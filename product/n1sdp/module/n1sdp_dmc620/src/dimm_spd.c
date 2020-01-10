/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dimm_spd.h>

#include <mod_n1sdp_dmc620.h>
#include <mod_n1sdp_i2c.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static bool multi_rank;
static float t_refi = .0000078;
static uint32_t dmc_clk_freq;
static float dmc_clk_period;
static int32_t dmc_clk_period_ps;

static struct ddr4_spd ddr4_dimm0;
static struct ddr4_spd ddr4_dimm1;

/*
 * Internal APIs used by SPD functions
 */

static int spd_read(struct mod_n1sdp_i2c_master_api_polled *i2c_api,
    int address, uint8_t *spd_data)
{
    char data[2] = {0};
    int i;
    int status;

    status = i2c_api->write((FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_I2C, 0)),
                            WRITE_PAGE0, data, SPD_W_TRANSFER_SIZE, SPD_STOP);
    if (status != FWK_SUCCESS)
        return status;

    for (i = SPD_PAGE0_START; i <= MAX_SPD_PAGE0; i++) {
        status = i2c_api->read((FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_I2C, 0)),
                               address, (char *)&spd_data[i],
                               SPD_R_TRANSFER_SIZE);
        if (status != FWK_SUCCESS)
            return status;
    }

    status = i2c_api->write((FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_I2C, 0)),
                            WRITE_PAGE1, data, SPD_W_TRANSFER_SIZE, SPD_STOP);
    if (status != FWK_SUCCESS)
        return status;

    for (i = SPD_PAGE1_START; i <= MAX_SPD_PAGE1; i++) {
        status = i2c_api->read((FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_I2C, 0)),
                               address, (char *)&spd_data[i],
                               SPD_R_TRANSFER_SIZE);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int chk_ddr4_dimms(unsigned int speed,
    struct ddr4_spd *dimm0, struct ddr4_spd *dimm1)
{
    uint8_t *dimm0_dram_param;
    uint8_t *dimm1_dram_param;

    fwk_assert((dimm0 != NULL) && (dimm1 != NULL));

    dimm0_dram_param = (uint8_t *)dimm0;
    dimm1_dram_param = (uint8_t *)dimm1;

    if (memcmp(dimm0_dram_param, dimm1_dram_param, 125) != 0)
        return FWK_E_DATA;

    switch (speed) {
    case 800:
        if ((dimm0_dram_param[18] > 0x0A) || (dimm1_dram_param[18] > 0x0A))
            return FWK_E_DATA;
        break;
    case 1200:
        if ((dimm0_dram_param[18] > 0x07) || (dimm1_dram_param[18] > 0x07))
            return FWK_E_DATA;
        break;
    case 1333:
        if ((dimm0_dram_param[18] > 0x06) || (dimm1_dram_param[18] > 0x06))
            return FWK_E_DATA;
        break;
    default:
        fwk_assert(false);
        break;
    }

    return FWK_SUCCESS;
}

static void dimm_device_data(uint8_t *spd_data, uint8_t dimm_id)
{
    unsigned int i;

    if (spd_data[2] == 0x0C) {
        FWK_LOG_INFO("    DIMM %d information:", dimm_id);
        FWK_LOG_INFO(
            "    Manufacturer ID = 0x%x 0x%x", spd_data[320], spd_data[321]);
        FWK_LOG_INFO("    Module part number = ");
        for (i = 329; i <= 348; i++)
            FWK_LOG_INFO("%c", spd_data[i]);

        FWK_LOG_INFO(
            "    Module serial number = 0x%x 0x%x 0x%x 0x%x",
            spd_data[325],
            spd_data[326],
            spd_data[327],
            spd_data[328]);

        FWK_LOG_INFO(
            "    Module manufacturing week %d%d year %d%d",
            0xF & (spd_data[324] >> 4),
            0xF & spd_data[324],
            0xF & (spd_data[323] >> 4),
            0xF & spd_data[323]);
    } else {
        FWK_LOG_INFO("[DDR] ERROR! DDR4 SPD EEPROM Not Detected");
        fwk_assert(false);
    }
}

static int get_dimm_rank_bits(uint8_t element, uint32_t *temp_reg,
                       struct dimm_info *ddr)
{
    uint8_t pkg_rank = (element & SPD_PKG_RANK_BITS_MASK) >>
                       SPD_PKG_RANK_BITS_OFFSET;

    *temp_reg &= ~RANK_BITS_NEXT_MASK;

    switch (pkg_rank) {
    case SPD_PKG_RANK1:
        *temp_reg |= RANK_BITS_NEXT_0;
        multi_rank = false;
        ddr->ranks_to_train = 0x01;
        ddr->number_of_ranks = 1;
        break;
    case SPD_PKG_RANK2:
        *temp_reg |= RANK_BITS_NEXT_1;
        multi_rank = true;
        ddr->ranks_to_train = 0x03;
        ddr->number_of_ranks = 2;
        break;
    case SPD_PKG_RANK4:
        *temp_reg |= RANK_BITS_NEXT_2;
        multi_rank = true;
        ddr->ranks_to_train = 0x0F;
        ddr->number_of_ranks = 4;
        break;
    case SPD_PKG_RANK8:
        *temp_reg |= RANK_BITS_NEXT_3;
        multi_rank = true;
        ddr->ranks_to_train = 0xFF;
        ddr->number_of_ranks = 8;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_col_bits(uint8_t element, uint32_t *temp_reg)
{
    *temp_reg &= ~COL_BITS_NEXT_MASK;

    switch ((element & SPD_COL_ADDR_BITS_MASK)) {
    case SPD_COL_ADDR_BITS_10:
        *temp_reg |= COL_BITS_NEXT_10_BITS;
        break;
    case SPD_COL_ADDR_BITS_11:
        *temp_reg |= COL_BITS_NEXT_11_BITS;
        break;
    case SPD_COL_ADDR_BITS_12:
        *temp_reg |= COL_BITS_NEXT_12_BITS;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_row_bits(uint8_t element, uint32_t *temp_reg)
{
    *temp_reg &= ~ROW_BITS_NEXT_MASK;

    switch ((element & SPD_ROW_ADDR_BITS_MASK)) {
    case SPD_ROW_ADDR_BITS_12:
        *temp_reg |= ROW_BITS_NEXT_12_BITS;
        break;
    case SPD_ROW_ADDR_BITS_13:
        *temp_reg |= ROW_BITS_NEXT_13_BITS;
        break;
    case SPD_ROW_ADDR_BITS_14:
        *temp_reg |= ROW_BITS_NEXT_14_BITS;
        break;
    case SPD_ROW_ADDR_BITS_15:
        *temp_reg |= ROW_BITS_NEXT_15_BITS;
        break;
    case SPD_ROW_ADDR_BITS_16:
        *temp_reg |= ROW_BITS_NEXT_16_BITS;
        break;
    case SPD_ROW_ADDR_BITS_17:
        *temp_reg |= ROW_BITS_NEXT_17_BITS;
        break;
    case SPD_ROW_ADDR_BITS_18:
        *temp_reg |= ROW_BITS_NEXT_18_BITS;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_bank_addr_grp_bits(uint8_t element, uint32_t *temp_reg)
{
    uint8_t bank_addr = element & SPD_BANK_BITS_MASK;
    uint8_t bank_group = element & SPD_BANK_GROUP_BITS_MASK;

    *temp_reg &= ~BANK_BITS_NEXT_MASK;

    switch (bank_group) {
    case SPD_BANK_GROUP_BITS_0:
        switch (bank_addr) {
        case SPD_BANK_BITS_3:
            *temp_reg |= BANK_BITS_NEXT_8_BANKS;
            break;
        default:
            return FWK_E_PARAM;
        }
        break;
    case SPD_BANK_GROUP_BITS_2:
        switch (bank_addr) {
        case SPD_BANK_BITS_2:
            *temp_reg |= BANK_BITS_NEXT_8_BANKS;
            break;
        case SPD_BANK_BITS_3:
            *temp_reg |= BANK_BITS_NEXT_16_BANKS;
            break;
        default:
            return FWK_E_PARAM;
        }
        break;
    case SPD_BANK_GROUP_BITS_4:
        switch (bank_addr) {
        case SPD_BANK_BITS_2:
            *temp_reg |= BANK_BITS_NEXT_16_BANKS;
            break;
        default:
            return FWK_E_PARAM;
        }
        break;
    default:
        return FWK_E_PARAM;
    }

  return FWK_SUCCESS;
}

static int get_dimm_memory_device_width_next_bits(uint8_t element,
                                           uint32_t *temp_reg,
                                           struct dimm_info *ddr)
{
    uint8_t sdram_device_width = (element & SDRAM_DEVICE_WIDTH_MASK);

    *temp_reg &= ~MEM_DEV_WIDTH_NEXT_MASK;

    switch (sdram_device_width) {
    case SDRAM_DEVICE_WIDTH_0:
        *temp_reg |= MEM_DEV_WIDTH_NEXT_4;
        ddr->dimm_mem_width = 4;
        break;
    case SDRAM_DEVICE_WIDTH_1:
        *temp_reg |= MEM_DEV_WIDTH_NEXT_8;
        ddr->dimm_mem_width = 8;
        break;
    case SDRAM_DEVICE_WIDTH_2:
        *temp_reg |= MEM_DEV_WIDTH_NEXT_16;
        ddr->dimm_mem_width = 16;
        break;
    default:
        ddr->dimm_mem_width = 0;
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_cid_bits(uint8_t element, uint32_t *temp_reg)
{
    uint8_t pkg_type = element & SPD_PKG_TYPE_BITS_MASK;
    uint8_t die_cnt = element & SPD_DIE_CNT_BITS_MASK;
    uint8_t sig_ld = element & SPD_SIG_LOAD_BITS_MASK;

    *temp_reg &= ~CID_BITS_NEXT_MASK;

    switch (pkg_type) {
    case SPD_PKG_TYPE_BITS_0:
        switch (die_cnt) {
        case SPD_DIE_CNT_BITS_0:
            switch (sig_ld) {
            case SPD_SIG_LOAD_BITS_0:
                *temp_reg |= CID_BITS_NEXT_0_CID;
                break;
            default:
                return FWK_E_PARAM;
            }
        break;
        default:
            return FWK_E_PARAM;
        }
        break;
    case SPD_PKG_TYPE_BITS_1:
        switch (sig_ld) {
        case SPD_SIG_LOAD_BITS_1:
            *temp_reg |= CID_BITS_NEXT_0_CID;
            break;
        case SPD_SIG_LOAD_BITS_2:
            switch (die_cnt) {
            case SPD_DIE_CNT_BITS_2:
                *temp_reg |= CID_BITS_NEXT_1_CID;
                break;
            case SPD_DIE_CNT_BITS_4:
                *temp_reg |= CID_BITS_NEXT_2_CID;
                break;
            case SPD_DIE_CNT_BITS_8:
                *temp_reg |= CID_BITS_NEXT_3_CID;
                break;
            default:
                return FWK_E_PARAM;
            }
            break;
        default:
            return FWK_E_PARAM;
        }
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_memory_type_nxt(uint8_t element, uint32_t *temp_reg)
{
    uint8_t mem_type = element;

    *temp_reg &= ~DMC620_MEM_TYPE_MASK;

    switch (mem_type) {
    case SPD_DDR4_SDRAM:
        *temp_reg |= DMC620_MEM_TYPE_DDR4;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_memory_width(uint8_t element, uint32_t *temp_reg)
{
    uint8_t pri_bus_width = element & SPD_PRI_BUS_WIDTH_BITS_MASK;

    *temp_reg &= ~MEM_WIDTH_MASK;

    switch (pri_bus_width) {
    case SPD_PRI_BUS_WIDTH_BITS_32:
        *temp_reg |= MEM_WIDTH_64;
        break;
    case SPD_PRI_BUS_WIDTH_BITS_64:
        *temp_reg |= MEM_WIDTH_128;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int get_dimm_bank_grp_bits(uint8_t element, uint32_t *temp_reg)
{
    uint8_t bank_group = element & SPD_BANK_GROUP_BITS_MASK;

    *temp_reg &= ~MEM_BANK_GROUPS_NEXT_MASK;

    switch (bank_group) {
    case SPD_BANK_GROUP_BITS_0:
        *temp_reg |= MEM_BANK_GROUPS_NEXT_0;
        break;
    case SPD_BANK_GROUP_BITS_2:
        *temp_reg |= MEM_BANK_GROUPS_NEXT_2;
        break;
    case SPD_BANK_GROUP_BITS_4:
        *temp_reg |= MEM_BANK_GROUPS_NEXT_4;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static uint32_t cal_dly_wth_rounding(int32_t spd_val, int32_t spd_val_fine)
{
    int32_t tDLY = 0;
    int32_t tempNck = 0;
    int32_t tDLY_int_nck = 0;

    tDLY = ((spd_val * MTB) + (spd_val_fine * FTB));
    tempNck = (tDLY * 1000) / dmc_clk_period_ps;
    tempNck = tempNck + 974;
    tDLY_int_nck = (int32_t)(tempNck / 1000);

    return tDLY_int_nck;
}

/*
 * APIs invoked by DMC-620 core functions
 */
int dimm_spd_init_check(struct mod_n1sdp_i2c_master_api_polled *i2c_api,
                         struct dimm_info *ddr)
{
    int status;

    spd_read(i2c_api, DIMM0_SPD_SLAVE, (uint8_t *)&ddr4_dimm0);
    spd_read(i2c_api, DIMM1_SPD_SLAVE, (uint8_t *)&ddr4_dimm1);

    status = chk_ddr4_dimms(ddr->speed, &ddr4_dimm0, &ddr4_dimm1);
    if (status != FWK_SUCCESS)
        return status;

    dmc_clk_freq = ddr->speed * UINT32_C(1000000);
    dmc_clk_period = 1.0f / dmc_clk_freq;
    dmc_clk_period_ps = dmc_clk_period * 1000000000000.0f;

    return FWK_SUCCESS;
}

void dimm_spd_mem_info(void)
{
    dimm_device_data((uint8_t *)&ddr4_dimm0, 0);
    dimm_device_data((uint8_t *)&ddr4_dimm1, 1);
}

int dimm_spd_address_control(uint32_t *temp_reg, struct dimm_info *ddr)
{
    int status;
    uint8_t temp = 0;

    temp = ddr4_dimm0.dram_param.sdram_addr;
    status = get_dimm_col_bits(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    status = get_dimm_row_bits(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    temp = ddr4_dimm0.dram_param.sdram_density_banks;
    status = get_dimm_bank_addr_grp_bits(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    temp = ddr4_dimm0.dram_param.mod_org;
    status = get_dimm_rank_bits(temp, temp_reg, ddr);
    if (status != FWK_SUCCESS)
        return status;

    temp = ddr4_dimm0.dram_param.sdram_pkg_type;
    status = get_dimm_cid_bits(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

int dimm_spd_format_control(uint32_t *temp_reg)
{
    uint32_t status;
    uint8_t temp = 0;

    temp = ddr4_dimm0.dram_param.mod_mem_bus_width;
    status = get_dimm_memory_width(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

int dimm_spd_memory_type(uint32_t *temp_reg, struct dimm_info *ddr)
{
    uint32_t status;
    uint8_t temp = 0;

    temp = ddr4_dimm0.dram_param.kb_dram_type;
    status = get_dimm_memory_type_nxt(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    status = get_dimm_row_bits(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    temp = ddr4_dimm0.dram_param.sdram_density_banks;
    status = get_dimm_bank_grp_bits(temp, temp_reg);
    if (status != FWK_SUCCESS)
        return status;

    temp = ddr4_dimm0.dram_param.mod_org;
    status = get_dimm_memory_device_width_next_bits(temp, temp_reg, ddr);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

int dimm_spd_t_refi(uint32_t *temp_reg)
{
    float refi_tmp = t_refi/8;
    uint32_t tmp_refi_now = 0;

    *temp_reg &= ~T_REFI_NEXT_MASK;

    tmp_refi_now = (uint32_t)(refi_tmp/dmc_clk_period);
    *temp_reg = (T_REFI_NEXT_MASK & tmp_refi_now);

    return FWK_SUCCESS;
}

int dimm_spd_t_rfc(uint32_t *temp_reg)
{
    uint32_t status = FWK_SUCCESS;
    uint32_t tmp_value = 0;
    uint32_t rfc_tmp = 0;

    *temp_reg &= ~T_RFC_NEXT_MASK;
    rfc_tmp = (uint32_t)ddr4_dimm0.dram_param.trfc1min_msb;

    rfc_tmp = rfc_tmp << 8;
    rfc_tmp |= (uint32_t)ddr4_dimm0.dram_param.trfc1min_lsb;

    tmp_value = cal_dly_wth_rounding(rfc_tmp, 0);
    *temp_reg |= (T_RFC_NEXT_MASK & tmp_value);
    *temp_reg &= ~T_RFCFG_NEXT_MASK;
    *temp_reg |= (T_RFCFG_NEXT_MASK & (tmp_value << 10));
    *temp_reg &= ~T_RFCFC_NEXT_MASK;

    if (multi_rank == true) {
        status = dimm_spd_t_refi(&tmp_value);
        if (status != FWK_SUCCESS)
            return status;

        *temp_reg |= (T_RFCFC_NEXT_MASK &
                     ((uint32_t)((float)tmp_value/t_refi) << 20));
    }

    return FWK_SUCCESS;
}

int dimm_spd_t_rcd(uint32_t *temp_reg)
{
    uint8_t spd_tmp = 0;
    uint8_t spd_tmp2 = 0;
    uint32_t tmp_value = 0;

    *temp_reg &= ~T_RCD_NEXT_MASK;

    spd_tmp = ddr4_dimm0.dram_param.trcdmin;
    spd_tmp2 = ddr4_dimm0.dram_param.trcdmin_fine;

    tmp_value = cal_dly_wth_rounding((int32_t)spd_tmp, (int32_t)spd_tmp2);
    *temp_reg = T_RCD_NEXT_MASK & tmp_value;

    return FWK_SUCCESS;
}

int dimm_spd_t_ras(uint32_t *temp_reg)
{
    uint8_t temp = 0;
    uint32_t tras_tmp = 0;
    uint32_t tmp_value = 0;

    *temp_reg &= ~T_RAS_NEXT_MASK;
    tras_tmp = (uint32_t)ddr4_dimm0.dram_param.uppr_nbls_trasmin_trcmin;

    tras_tmp &= (uint32_t)LWR_NBBL_MASK;
    tras_tmp = tras_tmp << 8;

    temp = ddr4_dimm0.dram_param.trasmin_lsb;
    tras_tmp |= (uint32_t)temp;

    tmp_value = cal_dly_wth_rounding(tras_tmp, 0);
    *temp_reg = (T_RAS_NEXT_MASK & tmp_value);

    return FWK_SUCCESS;
}

int dimm_spd_t_rp(uint32_t *temp_reg)
{
    uint8_t spd_tmp = 0;
    uint8_t spd_tmp2 = 0;
    uint32_t tmp_value = 0;

    *temp_reg &= ~T_RP_NEXT_MASK;

    spd_tmp = ddr4_dimm0.dram_param.trpmin;
    spd_tmp2 = ddr4_dimm0.dram_param.trpmin_fine;

    tmp_value = cal_dly_wth_rounding((int32_t)spd_tmp, (int32_t)spd_tmp2);
    *temp_reg = T_RP_NEXT_MASK & tmp_value;

    return FWK_SUCCESS;
}

int dimm_spd_t_rrd(uint32_t *temp_reg)
{
    uint8_t spd_tmp = 0;
    uint8_t spd_tmp2 = 0;
    uint32_t tmp_value = 0;

    *temp_reg &= ~T_RRD_S_NEXT_MASK;

    spd_tmp = ddr4_dimm0.dram_param.trrd_smin;
    spd_tmp2 = ddr4_dimm0.dram_param.trrd_smin_fine;

    tmp_value = cal_dly_wth_rounding((int32_t)spd_tmp, (int32_t)spd_tmp2);
    *temp_reg = T_RRD_S_NEXT_MASK & tmp_value;
    *temp_reg &= ~T_RRD_L_NEXT_MASK;

    spd_tmp = ddr4_dimm0.dram_param.trrd_lmin;
    spd_tmp2 = ddr4_dimm0.dram_param.trrd_lmin_fine;

    tmp_value = 0;
    tmp_value = cal_dly_wth_rounding((int32_t)spd_tmp, (int32_t)spd_tmp2);
    tmp_value = tmp_value << 8;

    *temp_reg |= (T_RRD_L_NEXT_MASK & tmp_value);
    tmp_value = 0x04000000;
    *temp_reg |= (T_RRD_DLR_NEXT_MASK & tmp_value);

    return FWK_SUCCESS;
}

int dimm_spd_t_wtr(uint32_t* temp_reg, struct dimm_info *ddr)
{
    uint8_t spd_tmp1 = 0;
    uint8_t spd_temp = 0;
    uint32_t twtr_tmp = 0;
    uint32_t tmp_value = 0;
    uint32_t bl_value = 8;

    spd_temp = ddr4_dimm0.dram_param.twtrmin_un;
    twtr_tmp = (uint32_t)(spd_temp & LWR_NBBL_MASK);
    twtr_tmp = twtr_tmp << 8;

    spd_tmp1 = ddr4_dimm0.dram_param.twtr_smin;
    twtr_tmp |= (uint32_t)spd_tmp1;

    tmp_value = cal_dly_wth_rounding(twtr_tmp, 0);
    tmp_value = ddr->cwl_value + (bl_value/2) + tmp_value;
    *temp_reg = (T_WTR_S_NEXT_MASK & tmp_value);

    twtr_tmp = (uint32_t)(spd_temp & UPPR_NBBL_MASK);
    twtr_tmp = twtr_tmp << 4;

    spd_tmp1 = ddr4_dimm0.dram_param.twtr_lmin;
    twtr_tmp |= (uint32_t)spd_tmp1;
    tmp_value = cal_dly_wth_rounding(twtr_tmp, 0);
    tmp_value = ddr->cwl_value + (bl_value/2) + tmp_value;
    *temp_reg |= (T_WTR_L_NEXT_MASK & (tmp_value << 8));
    *temp_reg |= (T_WTR_CS_NEXT_MASK & (tmp_value << 16));

    return FWK_SUCCESS;
}

int dimm_spd_t_act_window(uint32_t *temp_reg)
{
    uint8_t temp = 0;
    uint32_t tfawmin = 0;
    uint32_t tmp_value = 0;
    uint8_t tmp_mac = 0;

    *temp_reg &= ~T_FAW_NEXT_MASK;

    tfawmin = (uint32_t)ddr4_dimm0.dram_param.tfawmin_msn;
    tfawmin &= (uint32_t)LWR_NBBL_MASK;
    tfawmin = tfawmin << 8;
    temp = ddr4_dimm0.dram_param.tfawmin_lsb;

    tfawmin |= temp;
    tmp_value = cal_dly_wth_rounding(tfawmin, 0);
    *temp_reg = (T_FAW_NEXT_MASK & tmp_value);
    *temp_reg &= ~T_FAW_DLR_NEXT_MASK;
    *temp_reg |= (T_FAW_DLR_NEXT_MASK & (tmp_value << 8));

    temp = ddr4_dimm0.dram_param.sdram_opt_features;
    tmp_mac = ~SPD_T_MAC_MASK & temp;

    switch (tmp_mac) {
    case SPD_T_MAC_0:
    case SPD_T_MAC_1:
    case SPD_T_MAC_2:
    case SPD_T_MAC_3:
    case SPD_T_MAC_4:
    case SPD_T_MAC_5:
    case SPD_T_MAC_6:
    case SPD_T_MAC_7:
    case SPD_T_MAC_8:
      *temp_reg &= ~T_MAWI_NEXT_MASK;
      break;
    default:
      return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

int dimm_spd_calculate_dimm_size_gb(uint32_t *size_gb)
{
    uint64_t size;
    uint8_t temp;

    size = 0;
    temp = ddr4_dimm0.dram_param.sdram_density_banks;
    temp = (temp & SPD_SDRAM_DENSITY_MASK) >> SPD_SDRAM_DENSITY_POS;

    if (temp <= 7)
        size = (uint64_t)(1 << temp) * 256UL * FWK_MIB;
    else if (temp == 0x8)
        size = 12UL * FWK_GIB;
    else if (temp == 0x9)
        size = 24UL * FWK_GIB;
    else
        return FWK_E_DEVICE;

    size = size / 8UL;
    temp = ddr4_dimm0.dram_param.mod_mem_bus_width;
    temp = (temp & SPD_PRI_BUS_WIDTH_BITS_MASK) >> SPD_PRI_BUS_WIDTH_BITS_POS;
    if (temp <= 3)
        size = size * (uint64_t)((1 << temp) * 8UL);
    else
        return FWK_E_DEVICE;

    temp = ddr4_dimm0.dram_param.mod_org;
    temp = (temp & SDRAM_DEVICE_WIDTH_MASK) >> SDRAM_DEVICE_WIDTH_POS;
    if (temp <= 3)
        size = size / (uint64_t)((1 << temp) * 4);
    else
        return FWK_E_DEVICE;

    temp = ddr4_dimm0.dram_param.mod_org;
    temp = (temp & SPD_PKG_RANK_BITS_MASK) >> SPD_PKG_RANK_BITS_OFFSET;
    if (temp <= 7)
        size = size * (uint64_t)(temp + 1);
    else
        return FWK_E_DEVICE;

    *size_gb = size / FWK_GIB;
    return FWK_SUCCESS;
}
