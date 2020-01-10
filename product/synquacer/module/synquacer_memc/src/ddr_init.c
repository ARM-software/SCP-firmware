/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_config.h"
#include "synquacer_ddr.h"

#include <boot_ctl.h>
#include <ddr_init.h>

#include <internal/reg_DDRPHY_CONFIG.h>
#include <internal/reg_DMC520.h>

#include <stdint.h>

extern void usleep_en(uint32_t usec);

int ddr_init_mc0_mp(REG_ST_DMC520 *REG_DMC520);
int ddr_init_phy0_mp(REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
                     int retention_en);
int ddr_init_phy1_mp(REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
                     int retention_en);
int ddr_init_sdram_mp(
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
    int retention_en);
int ddr_init_mc1_mp(REG_ST_DMC520 *REG_DMC520);
int ddr_init_train_mp(
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
    int retention_en);
int ddr_init_mc2_mp(REG_ST_DMC520 *REG_DMC520);

int g_DDR4_DMC520_INIT_CH = 0;

#define DDR_DIP_SW3_SECURE_DRAM_ENABLE (0x04)
uint8_t ddr_is_secure_dram_enabled(void)
{
    return get_dsw3_status(DDR_DIP_SW3_SECURE_DRAM_ENABLE);
}

void Wait_for_ddr(uint32_t count)
{
    uint32_t i;
    for (i = 0; i < count; i++)
        __NOP();
}

void usleep_en(uint32_t usec)
{
    dmb();
    usleep(usec);
    return;
}

int ddr_dual_ch_init_mp(void)
{
    int ret_val;
    REG_ST_DMC520 *REG_DMC520_0;
    REG_ST_DMC520 *REG_DMC520_1;

    ret_val = ddr_ch0_init_mp();
    if (ret_val != 0)
        return ret_val;

    ret_val = ddr_ch1_init_mp();
    if (ret_val != 0)
        return ret_val;

    REG_DMC520_0 = (REG_ST_DMC520 *)(uint32_t)REG_DMC520_0_BA;

    REG_DMC520_1 = (REG_ST_DMC520 *)(uint32_t)REG_DMC520_1_BA;
    REG_DMC520_0->memc_cmd = 0x00000000;

    ddr_wait(
        (REG_DMC520_0->memc_status & 0x7) != 0x0, DDR_WAIT_TIMEOUT_US, 0x1001);

    REG_DMC520_0->address_map_next = 0xFF000005;
    REG_DMC520_0->memc_status;
    REG_DMC520_0->memc_status;
    REG_DMC520_0->memc_status;
    REG_DMC520_0->memc_status;
    REG_DMC520_0->memc_cmd = 0x00000003;
    REG_DMC520_0->memc_status;
    REG_DMC520_0->memc_cmd = 0x00000004;

    ddr_wait(
        (REG_DMC520_0->memc_status & 0x7) != 0x3, DDR_WAIT_TIMEOUT_US, 0x1002);

    REG_DMC520_1->memc_cmd = 0x00000000;

    ddr_wait(
        (REG_DMC520_1->memc_status & 0x7) != 0x0, DDR_WAIT_TIMEOUT_US, 0x1003);

    REG_DMC520_1->address_map_next = 0xFF000005;
    REG_DMC520_1->memc_status;
    REG_DMC520_1->memc_status;
    REG_DMC520_1->memc_status;
    REG_DMC520_1->memc_status;
    REG_DMC520_1->memc_cmd = 0x00000003;
    REG_DMC520_1->memc_status;
    REG_DMC520_1->memc_cmd = 0x00000004;

    ddr_wait(
        (REG_DMC520_1->memc_status & 0x7) != 0x3, DDR_WAIT_TIMEOUT_US, 0x1004);

    return 0;
}

/* ch0 : DMC 0 + PHY 0 */
int ddr_ch0_init_mp(void)
{
    int status;
    REG_ST_DMC520 *REG_DMC520_0;
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG_0;

    REG_DMC520_0 = (REG_ST_DMC520 *)(uint32_t)REG_DMC520_0_BA;
    REG_DDRPHY_CONFIG_0 =
        (REG_ST_DDRPHY_CONFIG_t *)(uint32_t)REG_DDRPHY_CONFIG_0_BA;

    g_DDR4_DMC520_INIT_CH = 0;

    FWK_LOG_INFO("[DDR] Initializing DDR ch0");

    ddr_init_mc0_mp(REG_DMC520_0);

    /* allocate 60MiB secure DRAM for OP-TEE */
    if (ddr_is_secure_dram_enabled()) {
        FWK_LOG_INFO("[DDR] secure DRAM enabled");
        REG_DMC520_0->access_address_min0_31_00_next = 0xFC00000C;
        REG_DMC520_0->access_address_min0_43_32_next = 0x00000000;
        REG_DMC520_0->access_address_max0_31_00_next = 0xFFBF0000;
        REG_DMC520_0->access_address_max0_43_32_next = 0x00000000;
    }

    status = ddr_init_phy0_mp(REG_DDRPHY_CONFIG_0, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch0 initialize failed. ddr_init_phy0_mp()");
        return status;
    }
    status = ddr_init_phy1_mp(REG_DDRPHY_CONFIG_0, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch0 initialize failed. ddr_init_phy1_mp()");
        return status;
    }

    status = ddr_init_sdram_mp(REG_DDRPHY_CONFIG_0, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch0 initialize failed. ddr_init_sdram_mp()");
        return status;
    }

    status = ddr_init_mc1_mp(REG_DMC520_0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch0 initialize failed. ddr_init_mc1_mp()");
        return status;
    }

    status = ddr_init_train_mp(REG_DDRPHY_CONFIG_0, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch0 fatal error occurred.");
        return status;
    }
    status = ddr_init_mc2_mp(REG_DMC520_0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch0 initialize failed. ddr_init_mc1_mp()");
        return status;
    }

    FWK_LOG_INFO("[DDR] Finished initializing DDR ch0");

    return 0;
}

/* ch1 : DMC 1 + PHY 1 */
int ddr_ch1_init_mp(void)
{
    int status;
    REG_ST_DMC520 *REG_DMC520_1;
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG_1;

    REG_DMC520_1 = (REG_ST_DMC520 *)(uint32_t)REG_DMC520_1_BA;
    REG_DDRPHY_CONFIG_1 =
        (REG_ST_DDRPHY_CONFIG_t *)(uint32_t)REG_DDRPHY_CONFIG_1_BA;

    g_DDR4_DMC520_INIT_CH = 1;

    FWK_LOG_INFO("[DDR] Initializing DDR ch1");

    status = ddr_init_mc0_mp(REG_DMC520_1);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 initialize failed. ddr_init_mc0_mp()");
        return status;
    }

    status = ddr_init_phy0_mp(REG_DDRPHY_CONFIG_1, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 initialize failed. ddr_init_phy0_mp()");
        return status;
    }

    status = ddr_init_phy1_mp(REG_DDRPHY_CONFIG_1, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 initialize failed. ddr_init_phy1_mp()");
        return status;
    }

    status = ddr_init_sdram_mp(REG_DDRPHY_CONFIG_1, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 initialize failed. ddr_init_sdram_mp()");
        return status;
    }

    status = ddr_init_mc1_mp(REG_DMC520_1);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 initialize failed. ddr_init_mc1_mp()");
        return status;
    }

    status = ddr_init_train_mp(REG_DDRPHY_CONFIG_1, 0);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 fatal error occurred.");
        return status;
    }

    status = ddr_init_mc2_mp(REG_DMC520_1);
    if (status != 0) {
        FWK_LOG_CRIT("[DDR] ch1 initialize failed. ddr_init_mc2_mp()");
        return status;
    }

    FWK_LOG_INFO("[DDR] Finished initializing DDR ch1");

    return 0;
}

int ddr_init_mc0_mp(REG_ST_DMC520 *REG_DMC520)
{
    uint32_t ddr_memory_type;
    ddr_memory_type = fw_get_memory_type();

    REG_DMC520->memc_status;
    REG_DMC520->memc_config;

    REG_DMC520->address_control_next = fw_get_address_control_next();
    REG_DMC520->decode_control_next = 0x00000002;
    REG_DMC520->format_control = 0x00000003;
    REG_DMC520->address_map_next = 0xFF800004;
    REG_DMC520->memory_address_max_31_00_next = 0xFFFF001F;
    REG_DMC520->memory_address_max_43_32_next = 0x00000007;
    REG_DMC520->access_address_min0_31_00_next = 0x00000000;
    REG_DMC520->access_address_min0_43_32_next = 0x00000000;
    REG_DMC520->access_address_max0_31_00_next = 0x00000000;
    REG_DMC520->access_address_max0_43_32_next = 0x00000000;
    REG_DMC520->access_address_min1_31_00_next = 0x00000000;
    REG_DMC520->access_address_min1_43_32_next = 0x00000000;
    REG_DMC520->access_address_max1_31_00_next = 0x00000000;
    REG_DMC520->access_address_max1_43_32_next = 0x00000000;
    REG_DMC520->access_address_min2_31_00_next = 0x00000000;
    REG_DMC520->access_address_min2_43_32_next = 0x00000000;
    REG_DMC520->access_address_max2_31_00_next = 0x00000000;
    REG_DMC520->access_address_max2_43_32_next = 0x00000000;
    REG_DMC520->access_address_min3_31_00_next = 0x00000000;
    REG_DMC520->access_address_min3_43_32_next = 0x00000000;
    REG_DMC520->access_address_max3_31_00_next = 0x00000000;
    REG_DMC520->access_address_max3_43_32_next = 0x00000000;
    REG_DMC520->access_address_min4_31_00_next = 0x00000000;
    REG_DMC520->access_address_min4_43_32_next = 0x00000000;
    REG_DMC520->access_address_max4_31_00_next = 0x00000000;
    REG_DMC520->access_address_max4_43_32_next = 0x00000000;
    REG_DMC520->access_address_min5_31_00_next = 0x00000000;
    REG_DMC520->access_address_min5_43_32_next = 0x00000000;
    REG_DMC520->access_address_max5_31_00_next = 0x00000000;
    REG_DMC520->access_address_max5_43_32_next = 0x00000000;
    REG_DMC520->access_address_min6_31_00_next = 0x00000000;
    REG_DMC520->access_address_min6_43_32_next = 0x00000000;
    REG_DMC520->access_address_max6_31_00_next = 0x00000000;
    REG_DMC520->access_address_max6_43_32_next = 0x00000000;
    REG_DMC520->access_address_min7_31_00_next = 0x00000000;
    REG_DMC520->access_address_min7_43_32_next = 0x00000000;
    REG_DMC520->access_address_max7_31_00_next = 0x00000000;
    REG_DMC520->access_address_max7_43_32_next = 0x00000000;
    REG_DMC520->dci_replay_type_next = 0x00000002;
    REG_DMC520->dci_strb = 0x0000000F;
    REG_DMC520->refresh_control_next = 0x00000000;
    REG_DMC520->memory_type_next = fw_get_memory_type_next();
    REG_DMC520->feature_config = 0x00000000;
    REG_DMC520->scrub_control0_next = 0x08000000;
    REG_DMC520->scrub_control1_next = 0x08000000;
    REG_DMC520->scrub_control2_next = 0x08000000;
    REG_DMC520->scrub_control3_next = 0x08000000;
    REG_DMC520->scrub_control4_next = 0x08000000;
    REG_DMC520->scrub_control5_next = 0x08000000;
    REG_DMC520->scrub_control6_next = 0x08000000;
    REG_DMC520->scrub_control7_next = 0x08000000;

    if ((ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH))
        REG_DMC520->feature_control_next = 0x00A100F0;
    else if (
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DMC520->feature_control_next = 0x000000F8;
    else if (
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH))
        REG_DMC520->feature_control_next = 0x00A100F8;

    REG_DMC520->mux_control_next = 0x000000D0;

    if (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH)
        REG_DMC520->rank_remap_control_next = 0xFEDC10BA;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DMC520->rank_remap_control_next = 0xFEDC3210;
    else if (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH)
        REG_DMC520->rank_remap_control_next = 0xFEDCBA10;
    else if (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH)
        REG_DMC520->rank_remap_control_next = 0xFEDCBA90;
    else if (
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DMC520->rank_remap_control_next = 0xFEDC90BA;

    REG_DMC520->t_refi_next = 0x0009040F;
    REG_DMC520->t_rfc_next = 0x0005D976;
    REG_DMC520->t_mrr_next = 0x00000001;
    REG_DMC520->t_mrw_next = 0x00180018;
    REG_DMC520->t_rdpden_next = 0x00000014;
    REG_DMC520->t_rcd_next = 0x00000010;
    REG_DMC520->t_ras_next = 0x00000024;
    REG_DMC520->t_rp_next = 0x00000010;
    REG_DMC520->t_rpall_next = 0x00000010;
    REG_DMC520->t_rrd_next = 0x00000604;
    REG_DMC520->t_act_window_next = 0x00000017;
    REG_DMC520->t_rtr_next = 0x000E0604;
    REG_DMC520->t_rtw_next = 0x000F0F0F;
    REG_DMC520->t_rtp_next = 0x00000009;
    REG_DMC520->t_wr_next = 0x00000021;
    REG_DMC520->t_wtr_next = 0x00181818;
    REG_DMC520->t_wtw_next = 0x000A0604;
    REG_DMC520->t_xmpd_next = 0x00000480;
    REG_DMC520->t_ep_next = 0x00000006;
    REG_DMC520->t_xp_next = 0x00100007;
    REG_DMC520->t_esr_next = 0x00000007;
    REG_DMC520->t_xsr_next = 0x03000180;
    REG_DMC520->t_esrck_next = 0x0000000B;
    REG_DMC520->t_ckxsr_next = 0x0000000B;
    REG_DMC520->t_cmd_next = 0x00000000;
    REG_DMC520->t_parity_next = 0x00001A00;
    REG_DMC520->t_zqcs_next = 0x00000080;
    REG_DMC520->t_rw_odt_clr_next = 0x0000000C;

    if ((ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)) {
        REG_DMC520->t_rddata_en_next = 0x0000000C;
        REG_DMC520->t_phyrdlat_next = 0x0000001E;
        REG_DMC520->t_phywrlat_next = 0x0000000A;
    } else if (
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH)) {
        REG_DMC520->t_rddata_en_next = 0x0000000B;
        REG_DMC520->t_phyrdlat_next = 0x00000022;
        REG_DMC520->t_phywrlat_next = 0x00000009;
    } else if (
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH)) {
        REG_DMC520->t_rddata_en_next = 0x0000000B;
        REG_DMC520->t_phyrdlat_next = 0x00000021;
        REG_DMC520->t_phywrlat_next = 0x00000009;
    }

    REG_DMC520->rdlvl_control_next = 0x00000000;
    REG_DMC520->rdlvl_mrs_next = 0x00000000;
    REG_DMC520->t_rdlvl_en_next = 0x00000000;
    REG_DMC520->t_rdlvl_rr_next = 0x00000000;
    REG_DMC520->wrlvl_control_next = 0x00000000;
    REG_DMC520->wrlvl_mrs_next = 0x00000000;
    REG_DMC520->t_wrlvl_en_next = 0x00000000;
    REG_DMC520->t_wrlvl_ww_next = 0x00000000;
    REG_DMC520->phy_power_control_next = 0x09999900;
    REG_DMC520->t_lpresp_next = 0x00000000;
    REG_DMC520->phy_update_control_next = 0x001F0000;
    REG_DMC520->odt_timing_next = 0x0C020800;
    REG_DMC520->t_odth_next = 0x00000008;

    if (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH)
        REG_DMC520->odt_wr_control_31_00_next = 0x08040000;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DMC520->odt_wr_control_31_00_next = 0x0A050A05;
    else if (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH)
        REG_DMC520->odt_wr_control_31_00_next = 0x00000201;
    else if (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH)
        REG_DMC520->odt_wr_control_31_00_next = 0x00000001;
    else if (
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DMC520->odt_wr_control_31_00_next = 0x00040000;

    REG_DMC520->odt_wr_control_63_32_next = 0x00000000;

    if ((ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DMC520->odt_rd_control_31_00_next = 0x00000000;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DMC520->odt_rd_control_31_00_next = 0x02010804;

    REG_DMC520->odt_rd_control_63_32_next = 0x00000000;

    if (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH) {
        // for bit swap on sever board
        if (g_DDR4_DMC520_INIT_CH == 0) {
            REG_DMC520->dq_map_control_15_00_next = 0x2D032E0D;
            REG_DMC520->dq_map_control_31_16_next = 0x2E0D0E24;
            REG_DMC520->dq_map_control_47_32_next = 0x2B152B15;
            REG_DMC520->dq_map_control_63_48_next = 0x35162C0B;
            REG_DMC520->dq_map_control_71_64_next = 0x00AA2E03;
        } else if (g_DDR4_DMC520_INIT_CH == 1) {
            REG_DMC520->dq_map_control_15_00_next = 0x240E2D0E;
            REG_DMC520->dq_map_control_31_16_next = 0x31120923;
            REG_DMC520->dq_map_control_47_32_next = 0x350C2B15;
            REG_DMC520->dq_map_control_63_48_next = 0x2C0B3616;
            REG_DMC520->dq_map_control_71_64_next = 0x00AA3118;
        }
    } else if (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) {
        // for bit swap on sever board
        if (g_DDR4_DMC520_INIT_CH == 0) {
            REG_DMC520->dq_map_control_15_00_next = 0x0B36042E;
            REG_DMC520->dq_map_control_31_16_next = 0x0C2C2D0D;
            REG_DMC520->dq_map_control_47_32_next = 0x2E0D0C2C;
            REG_DMC520->dq_map_control_63_48_next = 0x2E03162C;
            REG_DMC520->dq_map_control_71_64_next = 0x00AA2E0D;
        } else if (g_DDR4_DMC520_INIT_CH == 1) {
            REG_DMC520->dq_map_control_15_00_next = 0x162B032D;
            REG_DMC520->dq_map_control_31_16_next = 0x0A27370E;
            REG_DMC520->dq_map_control_47_32_next = 0x24040C2C;
            REG_DMC520->dq_map_control_63_48_next = 0x230E0C35;
            REG_DMC520->dq_map_control_71_64_next = 0x00AA3113;
        }
    } else {
        REG_DMC520->dq_map_control_15_00_next =
            fw_get_ddr4_sdram_dq_map_control(0);
        REG_DMC520->dq_map_control_31_16_next =
            fw_get_ddr4_sdram_dq_map_control(1);
        REG_DMC520->dq_map_control_47_32_next =
            fw_get_ddr4_sdram_dq_map_control(2);
        REG_DMC520->dq_map_control_63_48_next =
            fw_get_ddr4_sdram_dq_map_control(3);
        REG_DMC520->dq_map_control_71_64_next =
            (fw_get_ddr4_sdram_dq_map_control(4) | 0x00AA0000);
    }

    REG_DMC520->phy_rdwrdata_cs_mask_31_00 = 0x00000000;
    REG_DMC520->phy_rdwrdata_cs_mask_63_32 = 0x00000000;
    REG_DMC520->phy_request_cs_remap = 0x00000000;
    REG_DMC520->odt_cp_control_31_00_next = 0x08040201;
    REG_DMC520->odt_cp_control_63_32_next = 0x80402010;
    REG_DMC520->interrupt_control = 0x000003FF;

#ifdef DDR_DBI_ON
    REG_DMC520->t_rtw_next += 0x00030303;
    REG_DMC520->t_rtp_next += 0x00000003; // + tDBI=3clk
    REG_DMC520->t_rddata_en_next += 0x00000003;
    REG_DMC520->odt_timing_next += 0x03030000;
    REG_DMC520->t_rw_odt_clr_next += 0x00000003;
    REG_DMC520->feature_config |= 0x00000010;
    REG_DMC520->feature_control_next |= 0x00000005;
#endif // DDR_DBI_ON

#ifdef DDR_CA_Parity_ON
    dmb();
    REG_DMC520->t_mrw_next += 0x00040004;
    REG_DMC520->t_wr_next += 0x00000004;
    REG_DMC520->t_wtr_next += 0x00040404;
    REG_DMC520->t_esrck_next += 0x00000004;
    REG_DMC520->t_parity_next += 0x00000400;
    REG_DMC520->t_rddata_en_next += 0x00000004;
    REG_DMC520->t_phyrdlat_next += 0x00000004;
    REG_DMC520->t_phywrlat_next += 0x00000004;
#endif // DDR_CA_Parity_ON

#ifdef DDR_CRC_ON
    dmb();
    REG_DMC520->t_wr_next += 0x00000005;
    REG_DMC520->t_wtr_next += 0x00050505;
    REG_DMC520->t_wtw_next += 0x00000001;
    REG_DMC520->t_parity_next = 0x00002F00;
    REG_DMC520->odt_timing_next += 0x00000100;
    REG_DMC520->t_odth_next += 0x00000001;
    REG_DMC520->feature_control_next |= 0x00000002;
#endif // DDR_CRC_ON

    if (dram_ecc_is_enabled())
        REG_DMC520->feature_config |= 0x00000015;

    // UPDATE
    REG_DMC520->direct_cmd = 0x0001000C;

    // TRAIN
    REG_DMC520->direct_addr = 0x00000004;
    REG_DMC520->direct_cmd = 0x0001000A;

    // dfi_data_byte_disable control
    if (fw_get_ddr4_sdram_ecc_available()) {
        if ((REG_DMC520->feature_config & 0x3) == 0) {
            // [1:0] ecc_enable = b01 (ECC BYTE dfi_data_byte_disable=0)
            REG_DMC520->feature_config |= 0x00000001;
        }
    } else {
        // [1:0] ecc_enable = b00 (ECC BYTE dfi_data_byte_disable=1)
        REG_DMC520->feature_config &= 0xFFFFFFFC;
    }

    REG_DMC520->memc_status;
    REG_DMC520->memc_config;

    return 0;
}

int ddr_init_phy0_mp(
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
    int retention_en)
{
    uint32_t ddr_memory_type;
    ddr_memory_type = fw_get_memory_type();

    REG_DDRPHY_CONFIG->PGSR0;
    REG_DDRPHY_CONFIG->PGSR1;

    REG_DDRPHY_CONFIG->PGCR0 = 0x07D81E01;
    REG_DDRPHY_CONFIG->PGCR1 = 0x02004660;
    REG_DDRPHY_CONFIG->PGCR2 = 0x00012480;
    REG_DDRPHY_CONFIG->PTR0 = 0x42C21510;
    REG_DDRPHY_CONFIG->PTR1 = 0xD05612C0;
    REG_DDRPHY_CONFIG->PTR3 = 0x18082356;
    REG_DDRPHY_CONFIG->PTR4 = 0x10034156;
    REG_DDRPHY_CONFIG->PLLCR = 0x00038000;
    REG_DDRPHY_CONFIG->DXCCR = 0x20C00004;
    REG_DDRPHY_CONFIG->DSGCR = 0x0020403E;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH)) {
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000002;
        REG_DDRPHY_CONFIG->ODTCR = 0x00040000;
        REG_DDRPHY_CONFIG->DCR = 0x38000404;
    } else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH) {
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000000;
        REG_DDRPHY_CONFIG->ODTCR = 0x00050004;
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000001;
        REG_DDRPHY_CONFIG->ODTCR = 0x000A0008;
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000002;
        REG_DDRPHY_CONFIG->ODTCR = 0x00050001;
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000003;
        REG_DDRPHY_CONFIG->ODTCR = 0x000A0002;
        REG_DDRPHY_CONFIG->DCR = 0x08000404;
    } else if (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH) {
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000000;
        REG_DDRPHY_CONFIG->ODTCR = 0x00010000;
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000001;
        REG_DDRPHY_CONFIG->ODTCR = 0x00020000;
        REG_DDRPHY_CONFIG->DCR = 0x38000404;
    } else if (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) {
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000000;
        REG_DDRPHY_CONFIG->ODTCR = 0x00010000;
        REG_DDRPHY_CONFIG->DCR = 0x38000404;
    } else if (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) {
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000002;
        REG_DDRPHY_CONFIG->ODTCR = 0x00040000;
        REG_DDRPHY_CONFIG->RANKIDR = 0x00000003;
        REG_DDRPHY_CONFIG->ODTCR = 0x00080000;
        REG_DDRPHY_CONFIG->DCR = 0x08000404;
    }

    REG_DDRPHY_CONFIG->DTPR0 = 0x06241009;
    REG_DDRPHY_CONFIG->DTPR1 = 0x28270008;
    REG_DDRPHY_CONFIG->DTPR2 = 0x00060200;
    REG_DDRPHY_CONFIG->DTPR3 = 0x22800101;
    REG_DDRPHY_CONFIG->DTPR4 = 0x01760B07;
    REG_DDRPHY_CONFIG->DTPR5 = 0x00341008;
    REG_DDRPHY_CONFIG->DTPR6 = 0x00000505;

    if ((ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)) {
        if (retention_en == 0)
            REG_DDRPHY_CONFIG->RDIMMGCR0 = 0xBC410001;
        else
            REG_DDRPHY_CONFIG->RDIMMGCR0 = 0xBC410009;

        REG_DDRPHY_CONFIG->RDIMMGCR1 = 0x00001903;
        REG_DDRPHY_CONFIG->RDIMMGCR2 = 0x07FFFF3F;
        REG_DDRPHY_CONFIG->RDIMMCR0 = 0x00555100;
        REG_DDRPHY_CONFIG->RDIMMCR1 = 0x00C0A208;
        REG_DDRPHY_CONFIG->RDIMMCR2 = 0x002C0100;
        REG_DDRPHY_CONFIG->RDIMMCR3 = 0x00000000;
        REG_DDRPHY_CONFIG->RDIMMCR4 = 0x00070000;
    }

    REG_DDRPHY_CONFIG->MR0 = 0x00000830;
    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH)) {
        REG_DDRPHY_CONFIG->MR1 = 0x00000703;
        REG_DDRPHY_CONFIG->MR2 = 0x00000010;
    } else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH) {
        REG_DDRPHY_CONFIG->MR1 = 0x00000701;
        REG_DDRPHY_CONFIG->MR2 = 0x00000210;
    } else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH)) {
        REG_DDRPHY_CONFIG->MR1 = 0x00000701;
        REG_DDRPHY_CONFIG->MR2 = 0x00000010;
    }

    REG_DDRPHY_CONFIG->MR3 = 0x00000200;
    REG_DDRPHY_CONFIG->MR4 = 0x00000000;
    REG_DDRPHY_CONFIG->MR5 = 0x00000500;

    if ((ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->MR6 = 0x0000082B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->MR6 = 0x00000829;
    else if (
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->MR6 = 0x0000082D;

    REG_DDRPHY_CONFIG->DTCR0 = 0x8000B0C7;
    if (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH)
        REG_DDRPHY_CONFIG->DTCR1 = 0x000C2237;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DTCR1 = 0x000F0237;
    else if (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH)
        REG_DDRPHY_CONFIG->DTCR1 = 0x00030237;
    else if (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH)
        REG_DDRPHY_CONFIG->DTCR1 = 0x00010237;
    else if (
        (ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DTCR1 = 0x00042237;

    REG_DDRPHY_CONFIG->DTAR0 = 0x04000000;
    REG_DDRPHY_CONFIG->DTAR1 = 0x00010000;
    REG_DDRPHY_CONFIG->DTAR2 = 0x00030002;
    REG_DDRPHY_CONFIG->DCUTPR = 0x000000FF;
    REG_DDRPHY_CONFIG->BISTRR = 0x03E40000;
    REG_DDRPHY_CONFIG->BISTUDPR = 0xA5A5A5A5;
    REG_DDRPHY_CONFIG->RIOCR0 = 0x0FFF0000;
    REG_DDRPHY_CONFIG->RIOCR1 = 0xFF00FF00;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH)) {
        REG_DDRPHY_CONFIG->RIOCR2 = 0x0000008A;
        REG_DDRPHY_CONFIG->RIOCR4 = 0x0000008A;
        REG_DDRPHY_CONFIG->RIOCR5 = 0x0000008A;
        REG_DDRPHY_CONFIG->ACIOCR0 = 0x30003C10;
        REG_DDRPHY_CONFIG->ACIOCR1 = 0x00000000;
        REG_DDRPHY_CONFIG->ACIOCR3 = 0x0000008A;
    } else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH) {
        REG_DDRPHY_CONFIG->RIOCR2 = 0x00000000;
        REG_DDRPHY_CONFIG->RIOCR4 = 0x00000000;
        REG_DDRPHY_CONFIG->RIOCR5 = 0x00000000;
        REG_DDRPHY_CONFIG->ACIOCR0 = 0xF0003C10;
        REG_DDRPHY_CONFIG->ACIOCR1 = 0x00000000;
        REG_DDRPHY_CONFIG->ACIOCR3 = 0x00000000;
    } else if (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH) {
        REG_DDRPHY_CONFIG->RIOCR2 = 0x000000A0;
        REG_DDRPHY_CONFIG->RIOCR4 = 0x000000A0;
        REG_DDRPHY_CONFIG->RIOCR5 = 0x000000A0;
        REG_DDRPHY_CONFIG->ACIOCR0 = 0x30003C10;
        REG_DDRPHY_CONFIG->ACIOCR1 = 0x00000000;
        REG_DDRPHY_CONFIG->ACIOCR3 = 0x000000A0;
    } else if (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) {
        REG_DDRPHY_CONFIG->RIOCR2 = 0x000000A8;
        REG_DDRPHY_CONFIG->RIOCR4 = 0x000000A8;
        REG_DDRPHY_CONFIG->RIOCR5 = 0x000000A8;
        REG_DDRPHY_CONFIG->ACIOCR0 = 0x30003C10;
        REG_DDRPHY_CONFIG->ACIOCR1 = 0x00000000;
        REG_DDRPHY_CONFIG->ACIOCR3 = 0x000000A8;
    } else if (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) {
        REG_DDRPHY_CONFIG->RIOCR2 = 0x0000000A;
        REG_DDRPHY_CONFIG->RIOCR4 = 0x0000000A;
        REG_DDRPHY_CONFIG->RIOCR5 = 0x0000000A;
        REG_DDRPHY_CONFIG->ACIOCR0 = 0x30003C10;
        REG_DDRPHY_CONFIG->ACIOCR1 = 0x00000000;
        REG_DDRPHY_CONFIG->ACIOCR3 = 0x0000000A;
    }

    REG_DDRPHY_CONFIG->IOVCR0 = 0x0F090025;
    REG_DDRPHY_CONFIG->IOVCR1 = 0x00000109;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH)) {
        REG_DDRPHY_CONFIG->VTCR0 = 0x70030AAD;
        REG_DDRPHY_CONFIG->VTCR1 = 0x0FC0F076;
        REG_DDRPHY_CONFIG->ZQCR = 0x03058F00;
        REG_DDRPHY_CONFIG->ZQ0PR = 0x0001DD1D;
        REG_DDRPHY_CONFIG->ZQ1PR = 0x00079979;
        REG_DDRPHY_CONFIG->ZQ2PR = 0x000BDDBD;
        REG_DDRPHY_CONFIG->ZQ3PR = 0x000BDDBD;
    } else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH) {
        REG_DDRPHY_CONFIG->VTCR0 = 0x7002C9A9;
        REG_DDRPHY_CONFIG->VTCR1 = 0x0F77F076;
        REG_DDRPHY_CONFIG->ZQCR = 0x03058F00;
        REG_DDRPHY_CONFIG->ZQ0PR = 0x00019919;
        REG_DDRPHY_CONFIG->ZQ1PR = 0x0005DD5D;
        REG_DDRPHY_CONFIG->ZQ2PR = 0x00011111;
        REG_DDRPHY_CONFIG->ZQ3PR = 0x00011111;
    } else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH)) {
        REG_DDRPHY_CONFIG->VTCR0 = 0x7002EA2B;
        REG_DDRPHY_CONFIG->VTCR1 = 0x0FC0F076;
        REG_DDRPHY_CONFIG->ZQCR = 0x03058F00;
        REG_DDRPHY_CONFIG->ZQ0PR = 0x0001DD1D;
        REG_DDRPHY_CONFIG->ZQ1PR = 0x0009DD9D;
        REG_DDRPHY_CONFIG->ZQ2PR = 0x000BDDBD;
        REG_DDRPHY_CONFIG->ZQ3PR = 0x000BDDBD;
    }

    REG_DDRPHY_CONFIG->DX0GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX0GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX0GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX0GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX0GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX0GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX0GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX0GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX1GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX1GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX1GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX1GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX1GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX1GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX1GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX1GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX2GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX2GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX2GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX2GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX2GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX2GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX2GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX2GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX3GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX3GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX3GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX3GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX3GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX3GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX3GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX3GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX4GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX4GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX4GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX4GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX4GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX4GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX4GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX4GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX5GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX5GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX5GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX5GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX5GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX5GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX5GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX5GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX6GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX6GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX6GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX6GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX6GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX6GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX6GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX6GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX7GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX7GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX7GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX7GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX7GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX7GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX7GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX7GCR7 = 0x00810808;
    REG_DDRPHY_CONFIG->DX8GCR0 = 0x40000205;
    REG_DDRPHY_CONFIG->DX8GCR1 = 0xAAAA0000;
    REG_DDRPHY_CONFIG->DX8GCR2 = 0x00000000;
    REG_DDRPHY_CONFIG->DX8GCR3 = 0xFFFC0808;

    if ((ddr_memory_type == UDIMM_4GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == UDIMM_8GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX8GCR5 = 0x2B2B2B2B;
    else if (ddr_memory_type == RDIMM_16GBPERSLOT_2SLOTPERCH)
        REG_DDRPHY_CONFIG->DX8GCR5 = 0x3A3A3A3A;
    else if (
        (ddr_memory_type == RDIMM_16GBPERSLOT_1SLOTPERCH) ||
        (ddr_memory_type == SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH))
        REG_DDRPHY_CONFIG->DX8GCR5 = 0x32323232;

    REG_DDRPHY_CONFIG->DX8GCR7 = 0x00810808;

#ifdef DDR_DBI_ON
    REG_DDRPHY_CONFIG->PGCR3 =
        (REG_DDRPHY_CONFIG->PGCR3 & 0xF1FFFF1F) | 0x08000060; // [7:5]RDBICL=3
    REG_DDRPHY_CONFIG->MR0 = (REG_DDRPHY_CONFIG->MR0 & 0xFFFFFF8B) |
        0x00000040; // [6:5][2] CL +3 (15 -> 18)
    REG_DDRPHY_CONFIG->MR5 = (REG_DDRPHY_CONFIG->MR5 & 0xFFFFE3FF) | 0x00001800;
    REG_DDRPHY_CONFIG->DTCR0 =
        (REG_DDRPHY_CONFIG->DTCR0 & 0xFFFF3FFF) | 0x00004000; // DBI de-skew
#endif // DDR_DBI_ON

#ifdef DDR_CRC_ON
    REG_DDRPHY_CONFIG->MR2 = (REG_DDRPHY_CONFIG->MR2 & 0xFFFFEFFF) | 0x00001000;
#endif // DDR_CRC_ON

#ifdef DDR_CA_Parity_ON
    REG_DDRPHY_CONFIG->MR5 = (REG_DDRPHY_CONFIG->MR5 & 0xFFFFFFF8) | 0x00000001;
#endif // DDR_CA_Parity_ON

    if (fw_get_ddr4_sdram_ecc_available()) {
        REG_DDRPHY_CONFIG->DX8GCR0 = 0x40000205;
        REG_DDRPHY_CONFIG->DX8GCR1 = 0xAAAA0000;
        REG_DDRPHY_CONFIG->DX8GCR2 = 0x00000000;
        REG_DDRPHY_CONFIG->DX8GCR3 = 0xFFFC0808;
        REG_DDRPHY_CONFIG->DX8GCR7 = 0x00810808;
    } else {
        REG_DDRPHY_CONFIG->DX8GCR0 = 0x00023220;
        REG_DDRPHY_CONFIG->DX8GCR1 = 0x55550000;
        REG_DDRPHY_CONFIG->DX8GCR2 = 0xAAAAAAAA;
        REG_DDRPHY_CONFIG->DX8GCR3 = 0xC0FCA4A4;
        REG_DDRPHY_CONFIG->DX8GCR7 = 0x00E8A4A4;
    }

    return 0;
}

int ddr_init_phy1_mp(
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
    int retention_en)
{
    if (retention_en == 0)
        REG_DDRPHY_CONFIG->PIR =
            (REG_DDRPHY_CONFIG->PIR & 0xFFFFFF8C) | 0x00000073;
    else
        REG_DDRPHY_CONFIG->PIR =
            (REG_DDRPHY_CONFIG->PIR & 0xBFFFFF8C) | 0x40000071;

    // Wait by completion of PHY initialization.
    dmb();
    Wait_for_ddr(1);
    usleep_en(1);

    // [0]IDONE, [1]PLDONE, [2]DCDONE, [3]ZCDONE, [31]APLOCK
    ddr_wait(
        (REG_DDRPHY_CONFIG->PGSR0 & 0x8000000F) != 0x8000000F,
        DDR_WAIT_TIMEOUT_US,
        0x1001);

    REG_DDRPHY_CONFIG->PGSR0;
    REG_DDRPHY_CONFIG->PGSR1;

    return 0;
}

int ddr_init_sdram_mp(
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
    int retention_en)
{
    uint32_t mr6;
    uint32_t mr6_vref_training_on;
    uint32_t mr6_vref_training_off;
    uint32_t Addr_Invert;

    if (retention_en == 0) {
        REG_DDRPHY_CONFIG->PIR = (REG_DDRPHY_CONFIG->PIR & 0xFFF7FE7E) |
            0x00000181 | ((REG_DDRPHY_CONFIG->RDIMMGCR0 & 0x1) << 19);
        dmb();
        usleep_en(500);

        // Wait by completion of DRAM initialization.
        dmb();
        Wait_for_ddr(1);
        // [0]IDONE, [4]DIDONE
        ddr_wait(
            (REG_DDRPHY_CONFIG->PGSR0 & 0x00000011) != 0x00000011,
            DDR_WAIT_TIMEOUT_US,
            0x2001);

        Addr_Invert =
            (((REG_DDRPHY_CONFIG->RDIMMCR0 & 0x1) == 0) &&
             ((REG_DDRPHY_CONFIG->RDIMMGCR0 & 0x1) == 1));
        mr6 = REG_DDRPHY_CONFIG->MR6;
        mr6_vref_training_on =
            (mr6 & 0xFFFFFF7F) | (1 << 7); // [7]VrefDQ Training Enable = 1
        mr6_vref_training_off =
            (mr6 & 0xFFFFFF7F) | (0 << 7); // [7]VrefDQ Training Enable = 0

        REG_DDRPHY_CONFIG->SCHCR0 =
            (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFF0F) | 0x00000010; // [7:4]CMD

        // VrefDQ Training On
        REG_DDRPHY_CONFIG->SCHCR1 = (REG_DDRPHY_CONFIG->SCHCR1 & 0xF000000F) |
            ((mr6_vref_training_on << 8) | (0x1 << 6) | (0x2 << 4) |
             (0x1 << 2)); //[27:8]SCADDR,[7:6]SCBG,[5:4]SCBK,[2]ALLRANK
        REG_DDRPHY_CONFIG->SCHCR0 = (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFFF0) |
            0x00000001; // [3:0]SCHTRIG
        if (Addr_Invert == 1) { // for RDIMM B-side
            REG_DDRPHY_CONFIG->SCHCR1 =
                (REG_DDRPHY_CONFIG->SCHCR1 & 0xF000000F) |
                ((mr6_vref_training_on << 8) | (0x3 << 6) | (0x2 << 4) |
                 (0x1 << 2)); //[27:8]SCADDR,[7:6]SCBG,[5:4]SCBK,[2]ALLRANK
            REG_DDRPHY_CONFIG->SCHCR0 =
                (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFFF0) |
                0x00000001; // [3:0]SCHTRIG
        }
        // wait tVREFDQE //
        // New VrefDQ Value
        REG_DDRPHY_CONFIG->SCHCR1 = (REG_DDRPHY_CONFIG->SCHCR1 & 0xF000000F) |
            ((mr6_vref_training_on << 8) | (0x1 << 6) | (0x2 << 4) |
             (0x1 << 2)); //[27:8]SCADDR,[7:6]SCBG,[5:4]SCBK,[2]ALLRANK
        REG_DDRPHY_CONFIG->SCHCR0 = (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFFF0) |
            0x00000001; // [3:0]SCHTRIG
        if (Addr_Invert == 1) { // for RDIMM B-side
            REG_DDRPHY_CONFIG->SCHCR1 =
                (REG_DDRPHY_CONFIG->SCHCR1 & 0xF000000F) |
                ((mr6_vref_training_on << 8) | (0x3 << 6) | (0x2 << 4) |
                 (0x1 << 2)); //[27:8]SCADDR,[7:6]SCBG,[5:4]SCBK,[2]ALLRANK
            REG_DDRPHY_CONFIG->SCHCR0 =
                (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFFF0) |
                0x00000001; // [3:0]SCHTRIG
        }
        // wait tVREFDQE //
        // VrefDQ Training Off
        REG_DDRPHY_CONFIG->SCHCR1 = (REG_DDRPHY_CONFIG->SCHCR1 & 0xF000000F) |
            ((mr6_vref_training_off << 8) | (0x1 << 6) | (0x2 << 4) |
             (0x1 << 2)); //[27:8]SCADDR,[7:6]SCBG,[5:4]SCBK,[2]ALLRANK
        REG_DDRPHY_CONFIG->SCHCR0 = (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFFF0) |
            0x00000001; // [3:0]SCHTRIG
        if (Addr_Invert == 1) { // for RDIMM B-side
            REG_DDRPHY_CONFIG->SCHCR1 =
                (REG_DDRPHY_CONFIG->SCHCR1 & 0xF000000F) |
                ((mr6_vref_training_off << 8) | (0x3 << 6) | (0x2 << 4) |
                 (0x1 << 2)); //[27:8]SCADDR,[7:6]SCBG,[5:4]SCBK,[2]ALLRANK
            REG_DDRPHY_CONFIG->SCHCR0 =
                (REG_DDRPHY_CONFIG->SCHCR0 & 0xFFFFFFF0) |
                0x00000001; // [3:0]SCHTRIGy
        }
    }

    REG_DDRPHY_CONFIG->PGSR0;
    dmb();
    usleep_en(500);
    REG_DDRPHY_CONFIG->PGSR1;
    dmb();
    usleep_en(500);

    return 0;
}

int ddr_init_mc1_mp(REG_ST_DMC520 *REG_DMC520)
{
    REG_DMC520->memc_status;
    REG_DMC520->memc_config;

    // POWERDOWN_ENTRY
    REG_DMC520->direct_addr = 0x00000006;
    REG_DMC520->direct_cmd = 0x000F0004;
    REG_DMC520->memc_status;

    // wait
    usleep_en(500);

    // INVALIDATE RESET
    REG_DMC520->direct_addr = 0x00000000;
    REG_DMC520->direct_cmd = 0x0001000B;
    REG_DMC520->memc_status;

    // wait
    usleep_en(500);

    // INVALIDATE RESET
    REG_DMC520->direct_addr = 0x00000001;
    REG_DMC520->direct_cmd = 0x000F000B;
    REG_DMC520->memc_status;

    // wait
    usleep_en(500);

    // WAIT
    REG_DMC520->direct_addr = 0x000003E8;
    REG_DMC520->direct_cmd = 0x0001000D;
    REG_DMC520->memc_status;
    REG_DMC520->direct_addr = 0x00000258;
    REG_DMC520->direct_cmd = 0x0001000D;
    REG_DMC520->memc_status;

    // INVALIDATE RESET
    REG_DMC520->direct_addr = 0x00010001;
    REG_DMC520->direct_cmd = 0x000F000B;
    REG_DMC520->memc_status;

    // wait
    usleep_en(500);

    // WAIT
    REG_DMC520->direct_addr = 0x0000003C;
    REG_DMC520->direct_cmd = 0x0001000D;
    REG_DMC520->memc_status;

    // NOP
    REG_DMC520->direct_addr = 0x00000000;
    REG_DMC520->direct_cmd = 0x000F0000;
    REG_DMC520->memc_status;

    return 0;
}

int ddr_init_train_mp(
    REG_ST_DDRPHY_CONFIG_t *REG_DDRPHY_CONFIG,
    int retention_en)
{
    int status = 0;
    uint32_t phy_status_0;

    REG_DDRPHY_CONFIG->PGSR0;
    REG_DDRPHY_CONFIG->PGSR1;

    /////////////////////////////////////////////////////////////////////
    //  1. Write Leveling, Gate Training, Write Leveling Adjust
    /////////////////////////////////////////////////////////////////////
    REG_DDRPHY_CONFIG->PGCR3 =
        (REG_DDRPHY_CONFIG->PGCR3 & 0xFFFFFFE7) | 0x00000000;
    dmb();

#ifdef DDR_DQSTRAINWA_ON
    REG_DDRPHY_CONFIG->PIR = (REG_DDRPHY_CONFIG->PIR & 0xFFFFFDFE) | 0x00000201;
    dmb();
    Wait_for_ddr(1);
    dmb();
    // [0]IDONE, [5]WLDONE
    while (((REG_DDRPHY_CONFIG->PGSR0 & 0x00000021) != 0x00000021) &&
           ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) == 0))
        ;

    REG_DDRPHY_CONFIG->DXCCR =
        (REG_DDRPHY_CONFIG->DXCCR & 0xFFBFFFFF) | 0x00000000;
    dmb();

    REG_DDRPHY_CONFIG->PIR = (REG_DDRPHY_CONFIG->PIR & 0xFFFFFBFE) | 0x00000401;
    dmb();
    Wait_for_ddr(1);
    dmb();
    // [0]IDONE, [6]QSGDONE
    while (((REG_DDRPHY_CONFIG->PGSR0 & 0x00000041) != 0x00000041) &&
           ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) == 0))
        ;

    REG_DDRPHY_CONFIG->DXCCR =
        (REG_DDRPHY_CONFIG->DXCCR & 0xFFBFFFFF) | 0x00400000;
    dmb();

    REG_DDRPHY_CONFIG->PIR = (REG_DDRPHY_CONFIG->PIR & 0xFFFFF7FE) | 0x00000801;
    dmb();
    Wait_for_ddr(1);
    dmb();
    // [0]IDONE, [7]WLADONE
    while (((REG_DDRPHY_CONFIG->PGSR0 & 0x00000081) != 0x00000081) &&
           ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) == 0))
        ;

#else
    REG_DDRPHY_CONFIG->PIR = (REG_DDRPHY_CONFIG->PIR & 0xFFFFF1FE) | 0x00000E01;
    dmb();
    // Wait by completion of Data Training.
    Wait_for_ddr(1);
    dmb();

    // [0]IDONE, [5]WLDONE, [6]QSGDONE, [7]WLADONE
    ddr_wait(
        ((REG_DDRPHY_CONFIG->PGSR0 & 0x000000E1) != 0x000000E1) &&
            ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) == 0),
        DDR_WAIT_TIMEOUT_US,
        0x3001);

    if ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) != 0) {
        FWK_LOG_CRIT(
            "[DDR] error : Write Leveling, Gate Training, Write Leveling "
            "Adjust");
        status = 0x3002;
        goto ERROR_END;
    }
#endif

    /////////////////////////////////////////////////////////////////////
    //  2. Data Bit Deskew, Data Eye, Static Read
    /////////////////////////////////////////////////////////////////////
    REG_DDRPHY_CONFIG->PGCR0 =
        (REG_DDRPHY_CONFIG->PGCR0 & 0xFBFFFFFF) | 0x00000000;
    dmb();
    // Wait for more than 10cycle with reference to pclk.
    Wait_for_ddr(1);

    REG_DDRPHY_CONFIG->PGCR3 =
        (REG_DDRPHY_CONFIG->PGCR3 & 0xFFFFFFE7) | 0x00000008;
    dmb();

    // Wait for more than 10cycle with reference to pclk.
    Wait_for_ddr(1);

    REG_DDRPHY_CONFIG->PGCR0 =
        (REG_DDRPHY_CONFIG->PGCR0 & 0xFBFFFFFF) | 0x04000000;
    dmb();

    // Wait for more than 10cycle with reference to pclk.
    Wait_for_ddr(1);

    REG_DDRPHY_CONFIG->PIR = (REG_DDRPHY_CONFIG->PIR & 0xFFFE0FFE) | 0x0001F001;
    dmb();

    // Wait by completion of Data Training.
    Wait_for_ddr(1);
    // [0]IDONE, [8]RDDONE, [9]WDDONE, [10]REDONE, [11]WEDONE, [13]SRDDONE
    // [note] timeout : Data Bit Deskew, Data Eye, Static Read
    ddr_wait(
        ((REG_DDRPHY_CONFIG->PGSR0 & 0x00002F01) != 0x00002F01) &&
            ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) == 0),
        DDR_WAIT_TIMEOUT_US,
        0x3003);

    if ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) != 0) {
        FWK_LOG_CRIT("[DDR] error : Data Bit Deskew, Data Eye, Static Read");
        status = 0x3004;
        goto ERROR_END;
    }

    /////////////////////////////////////////////////////////////////////
    //  3. VREF Training
    /////////////////////////////////////////////////////////////////////
    if (retention_en == 0) {
        REG_DDRPHY_CONFIG->DTCR0 =
            (REG_DDRPHY_CONFIG->DTCR0 & 0x0FFFFFFF) | 0x00000000;
        dmb();

        REG_DDRPHY_CONFIG->PIR =
            (REG_DDRPHY_CONFIG->PIR & 0xFFFDFFFE) | 0x00020001;
        dmb();

        // 76 Wait by completion of Data Training.
        Wait_for_ddr(20);

        // [14]VDONE
        ddr_wait(
            ((REG_DDRPHY_CONFIG->PGSR0 & 0x00004001) != 0x00004001) &&
                ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) == 0),
            DDR_WAIT_TIMEOUT_US,
            0x3005);

        if ((REG_DDRPHY_CONFIG->PGSR0 & 0x4FF80000) != 0) {
            FWK_LOG_CRIT("[DDR] error : VREF Training");
            status = 0x3006;
            goto ERROR_END;
        }

        REG_DDRPHY_CONFIG->DTCR0 =
            (REG_DDRPHY_CONFIG->DTCR0 & 0x0FFFFFFF) | 0x80000000;
    }

ERROR_END:
    /////////////////////////////////////////////////////////////////////
    //  Check Status
    /////////////////////////////////////////////////////////////////////
    phy_status_0 = REG_DDRPHY_CONFIG->PGSR0;
    if (((phy_status_0 >> 19) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s VREF Training Error", __func__);
        status = 0x3010;
    }
    if (((phy_status_0 >> 20) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Impedance Calibration Error", __func__);
        status = 0x3011;
    }
    if (((phy_status_0 >> 21) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Write Leveling Error", __func__);
        status = 0x3012;
    }
    if (((phy_status_0 >> 22) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s DQS Gate Training Error", __func__);
        status = 0x3013;
    }
    if (((phy_status_0 >> 23) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Write Leveling Adjustment Error", __func__);
        status = 0x3014;
    }
    if (((phy_status_0 >> 24) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Read Bit Deskew Error", __func__);
        status = 0x3015;
    }
    if (((phy_status_0 >> 25) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Write Bit Deskew Error", __func__);
        status = 0x3016;
    }
    if (((phy_status_0 >> 26) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Read Eye Training Error", __func__);
        status = 0x3017;
    }
    if (((phy_status_0 >> 27) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Write Eye Training Error", __func__);
        status = 0x3018;
    }
    if (((phy_status_0 >> 30) & 0x1) == 0x1) {
        FWK_LOG_CRIT("[DDR] %s Static Read Error", __func__);
        status = 0x3019;
    }

    if (status != 0)
        return status;

#ifdef DDR_TRAINING_RESULT
    ddr_init_train_result(REG_DDRPHY_CONFIG);
#endif // DDR_TRAINING_RESULT

    REG_DDRPHY_CONFIG->PGCR1 =
        (REG_DDRPHY_CONFIG->PGCR1 & 0xFFFFFFBF) | 0x00000000;

    REG_DDRPHY_CONFIG->PGSR0;
    REG_DDRPHY_CONFIG->PGSR1;

    return 0;
}

int ddr_init_mc2_mp(REG_ST_DMC520 *REG_DMC520)
{
    REG_DMC520->memc_status;
    REG_DMC520->memc_config;

    if (!dram_ecc_is_enabled())
        REG_DMC520->feature_config &= 0xFFFFFFFC;

    // UPDATE
    REG_DMC520->direct_cmd = 0x0001000C;

    // ZQC
    REG_DMC520->direct_addr = 0x00000400;
    REG_DMC520->direct_cmd = 0x000F0005;
    dmb();
    REG_DMC520->memc_status;

    // Wait
    REG_DMC520->direct_addr = 0x000003ff;
    REG_DMC520->direct_cmd = 0x0001000d;
    dmb();
    REG_DMC520->memc_status;

    REG_DMC520->memc_status;
    REG_DMC520->memc_status;
    REG_DMC520->memc_status;
    REG_DMC520->memc_status;
    REG_DMC520->memc_cmd = 0x00000003;
    dmb();
    REG_DMC520->memc_status;
    REG_DMC520->memc_cmd = 0x00000004;
    dmb();

    ddr_wait(
        (REG_DMC520->memc_status & 0x7) != 0x3, DDR_WAIT_TIMEOUT_US, 0x4001);

    REG_DMC520->memc_status;
    REG_DMC520->memc_config;

    return 0;
}
