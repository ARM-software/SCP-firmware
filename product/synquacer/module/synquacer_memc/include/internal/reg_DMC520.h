/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_REG_DMC520_H
#define INTERNAL_REG_DMC520_H

#include <stdint.h>

typedef volatile struct {
    uint32_t memc_status; // 0x000
    uint32_t memc_config; // 0x004
    uint32_t memc_cmd; // 0x008
    uint32_t __RESERVED_0x000C_0x000C_[1];
    uint32_t address_control_next; // 0x010
    uint32_t decode_control_next; // 0x014
    uint32_t format_control; // 0x018
    uint32_t address_map_next; // 0x01C
    uint32_t low_power_control_next; // 0x020
    uint32_t __RESERVED_0x0024_0x0024_[1];
    uint32_t turnaround_control_next; // 0x028
    uint32_t hit_turnaround_control_next; // 0x02C
    uint32_t qos_class_control_next; // 0x030
    uint32_t escalation_control_next; // 0x034
    uint32_t qv_control_31_00_next; // 0x038
    uint32_t qv_control_63_32_next; // 0x03C
    uint32_t rt_control_31_00_next; // 0x040
    uint32_t rt_control_63_32_next; // 0x044
    uint32_t timeout_control_next; // 0x048
    uint32_t credit_control_next; // 0x04C
    uint32_t write_priority_control_31_00_next; // 0x050
    uint32_t write_priority_control_63_32_next; // 0x054
    uint32_t __RESERVED_0x0058_0x005C_[2];
    uint32_t queue_threshold_control_31_00_next; // 0x060
    uint32_t queue_threshold_control_63_32_next; // 0x064
    uint32_t __RESERVED_0x0068_0x0074_[4];
    uint32_t memory_address_max_31_00_next; // 0x078
    uint32_t memory_address_max_43_32_next; // 0x07C
    uint32_t access_address_min0_31_00_next; // 0x080
    uint32_t access_address_min0_43_32_next; // 0x084
    uint32_t access_address_max0_31_00_next; // 0x088
    uint32_t access_address_max0_43_32_next; // 0x08C
    uint32_t access_address_min1_31_00_next; // 0x090
    uint32_t access_address_min1_43_32_next; // 0x094
    uint32_t access_address_max1_31_00_next; // 0x098
    uint32_t access_address_max1_43_32_next; // 0x09C
    uint32_t access_address_min2_31_00_next; // 0x0A0
    uint32_t access_address_min2_43_32_next; // 0x0A4
    uint32_t access_address_max2_31_00_next; // 0x0A8
    uint32_t access_address_max2_43_32_next; // 0x0AC
    uint32_t access_address_min3_31_00_next; // 0x0B0
    uint32_t access_address_min3_43_32_next; // 0x0B4
    uint32_t access_address_max3_31_00_next; // 0x0B8
    uint32_t access_address_max3_43_32_next; // 0x0BC
    uint32_t access_address_min4_31_00_next; // 0x0C0
    uint32_t access_address_min4_43_32_next; // 0x0C4
    uint32_t access_address_max4_31_00_next; // 0x0C8
    uint32_t access_address_max4_43_32_next; // 0x0CC
    uint32_t access_address_min5_31_00_next; // 0x0D0
    uint32_t access_address_min5_43_32_next; // 0x0D4
    uint32_t access_address_max5_31_00_next; // 0x0D8
    uint32_t access_address_max5_43_32_next; // 0x0DC
    uint32_t access_address_min6_31_00_next; // 0x0E0
    uint32_t access_address_min6_43_32_next; // 0x0E4
    uint32_t access_address_max6_31_00_next; // 0x0E8
    uint32_t access_address_max6_43_32_next; // 0x0EC
    uint32_t access_address_min7_31_00_next; // 0x0F0
    uint32_t access_address_min7_43_32_next; // 0x0F4
    uint32_t access_address_max7_31_00_next; // 0x0F8
    uint32_t access_address_max7_43_32_next; // 0x0FC
    uint32_t channel_status; // 0x100
    uint32_t __RESERVED_0x0104_0x0104_[1];
    uint32_t direct_addr; // 0x108
    uint32_t direct_cmd; // 0x10C
    uint32_t dci_replay_type_next; // 0x110
    uint32_t __RESERVED_0x0114_0x0114_[1];
    uint32_t dci_strb; // 0x118
    uint32_t dci_data; // 0x11C
    uint32_t refresh_control_next; // 0x120
    uint32_t __RESERVED_0x0124_0x0124_[1];
    uint32_t memory_type_next; // 0x128
    uint32_t __RESERVED_0x012C_0x012C_[1];
    uint32_t feature_config; // 0x130
    uint32_t __RESERVED_0x0134_0x0134_[1];
    uint32_t nibble_failed_031_000; // 0x138
    uint32_t nibble_failed_063_032; // 0x13C
    uint32_t nibble_failed_095_064; // 0x140
    uint32_t nibble_failed_127_096; // 0x144
    uint32_t queue_allocate_control_031_000; // 0x148
    uint32_t queue_allocate_control_063_032; // 0x14C
    uint32_t queue_allocate_control_095_064; // 0x150
    uint32_t queue_allocate_control_127_096; // 0x154
    uint32_t ecc_errc_count_31_00; // 0x158
    uint32_t ecc_errc_count_63_32; // 0x15C
    uint32_t ecc_errd_count_31_00; // 0x160
    uint32_t ecc_errd_count_63_32; // 0x164
    uint32_t ram_err_count; // 0x168
    uint32_t link_err_count; // 0x16C
    uint32_t scrub_control0_next; // 0x170
    uint32_t scrub_address_min0_next; // 0x174
    uint32_t scrub_address_max0_next; // 0x178
    uint32_t __RESERVED_0x017C_0x017C_[1];
    uint32_t scrub_control1_next; // 0x180
    uint32_t scrub_address_min1_next; // 0x184
    uint32_t scrub_address_max1_next; // 0x188
    uint32_t __RESERVED_0x018C_0x018C_[1];
    uint32_t scrub_control2_next; // 0x190
    uint32_t scrub_address_min2_next; // 0x194
    uint32_t scrub_address_max2_next; // 0x198
    uint32_t __RESERVED_0x019C_0x019C_[1];
    uint32_t scrub_control3_next; // 0x1A0
    uint32_t scrub_address_min3_next; // 0x1A4
    uint32_t scrub_address_max3_next; // 0x1A8
    uint32_t __RESERVED_0x01AC_0x01AC_[1];
    uint32_t scrub_control4_next; // 0x1B0
    uint32_t scrub_address_min4_next; // 0x1B4
    uint32_t scrub_address_max4_next; // 0x1B8
    uint32_t __RESERVED_0x01BC_0x01BC_[1];
    uint32_t scrub_control5_next; // 0x1C0
    uint32_t scrub_address_min5_next; // 0x1C4
    uint32_t scrub_address_max5_next; // 0x1C8
    uint32_t __RESERVED_0x01CC_0x01CC_[1];
    uint32_t scrub_control6_next; // 0x1D0
    uint32_t scrub_address_min6_next; // 0x1D4
    uint32_t scrub_address_max6_next; // 0x1D8
    uint32_t __RESERVED_0x01DC_0x01DC_[1];
    uint32_t scrub_control7_next; // 0x1E0
    uint32_t scrub_address_min7_next; // 0x1E4
    uint32_t scrub_address_max7_next; // 0x1E8
    uint32_t __RESERVED_0x01EC_0x01EC_[1];
    uint32_t feature_control_next; // 0x1F0
    uint32_t mux_control_next; // 0x1F4
    uint32_t rank_remap_control_next; // 0x1F8
    uint32_t scrub_control_next; // 0x1FC
    uint32_t t_refi_next; // 0x200
    uint32_t t_rfc_next; // 0x204
    uint32_t t_mrr_next; // 0x208
    uint32_t t_mrw_next; // 0x20C
    uint32_t t_rdpden_next; // 0x210
    uint32_t __RESERVED_0x0214_0x0214_[1];
    uint32_t t_rcd_next; // 0x218
    uint32_t t_ras_next; // 0x21C
    uint32_t t_rp_next; // 0x220
    uint32_t t_rpall_next; // 0x224
    uint32_t t_rrd_next; // 0x228
    uint32_t t_act_window_next; // 0x22C
    uint32_t __RESERVED_0x0230_0x0230_[1];
    uint32_t t_rtr_next; // 0x234
    uint32_t t_rtw_next; // 0x238
    uint32_t t_rtp_next; // 0x23C
    uint32_t __RESERVED_0x0240_0x0240_[1];
    uint32_t t_wr_next; // 0x244
    uint32_t t_wtr_next; // 0x248
    uint32_t t_wtw_next; // 0x24C
    uint32_t __RESERVED_0x0250_0x0250_[1];
    uint32_t t_xmpd_next; // 0x254
    uint32_t t_ep_next; // 0x258
    uint32_t t_xp_next; // 0x25C
    uint32_t t_esr_next; // 0x260
    uint32_t t_xsr_next; // 0x264
    uint32_t t_esrck_next; // 0x268
    uint32_t t_ckxsr_next; // 0x26C
    uint32_t t_cmd_next; // 0x270
    uint32_t t_parity_next; // 0x274
    uint32_t t_zqcs_next; // 0x278
    uint32_t t_rw_odt_clr_next; // 0x27C
    uint32_t __RESERVED_0x0280_0x02FC_[32];
    uint32_t t_rddata_en_next; // 0x300
    uint32_t t_phyrdlat_next; // 0x304
    uint32_t t_phywrlat_next; // 0x308
    uint32_t __RESERVED_0x030C_0x030C_[1];
    uint32_t rdlvl_control_next; // 0x310
    uint32_t rdlvl_mrs_next; // 0x314
    uint32_t t_rdlvl_en_next; // 0x318
    uint32_t t_rdlvl_rr_next; // 0x31C
    uint32_t wrlvl_control_next; // 0x320
    uint32_t wrlvl_mrs_next; // 0x324
    uint32_t t_wrlvl_en_next; // 0x328
    uint32_t t_wrlvl_ww_next; // 0x32C
    uint32_t __RESERVED_0x0330_0x0344_[6];
    uint32_t phy_power_control_next; // 0x348
    uint32_t t_lpresp_next; // 0x34C
    uint32_t phy_update_control_next; // 0x350
    uint32_t __RESERVED_0x0354_0x0354_[1];
    uint32_t odt_timing_next; // 0x358
    uint32_t t_odth_next; // 0x35C
    uint32_t odt_wr_control_31_00_next; // 0x360
    uint32_t odt_wr_control_63_32_next; // 0x364
    uint32_t odt_rd_control_31_00_next; // 0x368
    uint32_t odt_rd_control_63_32_next; // 0x36C
    uint32_t temperature_readout; // 0x370
    uint32_t __RESERVED_0x0374_0x0374_[1];
    uint32_t training_status; // 0x378
    uint32_t update_status; // 0x37C
    uint32_t dq_map_control_15_00_next; // 0x380
    uint32_t dq_map_control_31_16_next; // 0x384
    uint32_t dq_map_control_47_32_next; // 0x388
    uint32_t dq_map_control_63_48_next; // 0x38C
    uint32_t dq_map_control_71_64_next; // 0x390
    uint32_t __RESERVED_0x0394_0x0394_[1];
    uint32_t rank_status; // 0x398
    uint32_t mode_change_status; // 0x39C
    uint32_t phy_rdwrdata_cs_mask_31_00; // 0x3A0
    uint32_t phy_rdwrdata_cs_mask_63_32; // 0x3A4
    uint32_t phy_request_cs_remap; // 0x3A8
    uint32_t __RESERVED_0x03AC_0x03AC_[1];
    uint32_t odt_cp_control_31_00_next; // 0x3B0
    uint32_t odt_cp_control_63_32_next; // 0x3B4
    uint32_t __RESERVED_0x03B8_0x03FC_[18];
    uint32_t user_status; // 0x400
    uint32_t __RESERVED_0x0404_0x0404_[1];
    uint32_t user_config0_next; // 0x408
    uint32_t user_config1_next; // 0x40C
    uint32_t user_config2; // 0x410
    uint32_t user_config3; // 0x414
    uint32_t __RESERVED_0x0418_0x04FC_[58];
    uint32_t interrupt_control; // 0x500
    uint32_t __RESERVED_0x0504_0x0504_[1];
    uint32_t interrupt_clr; // 0x508
    uint32_t __RESERVED_0x050C_0x050C_[1];
    uint32_t interrupt_status; // 0x510
    uint32_t __RESERVED_0x0514_0x0514_[1];
    uint32_t ram_ecc_errc_int_info_31_00; // 0x518
    uint32_t ram_ecc_errc_int_info_63_32; // 0x51C
    uint32_t ram_ecc_errd_int_info_31_00; // 0x520
    uint32_t ram_ecc_errd_int_info_63_32; // 0x524
    uint32_t dram_ecc_errc_int_info_31_00; // 0x528
    uint32_t dram_ecc_errc_int_info_63_32; // 0x52C
    uint32_t dram_ecc_errd_int_info_31_00; // 0x530
    uint32_t dram_ecc_errd_int_info_63_32; // 0x534
    uint32_t failed_access_int_info_31_00; // 0x538
    uint32_t failed_access_int_info_63_32; // 0x53C
    uint32_t failed_prog_int_info_31_00; // 0x540
    uint32_t failed_prog_int_info_63_32; // 0x544
    uint32_t link_err_int_info_31_00; // 0x548
    uint32_t link_err_int_info_63_32; // 0x54C
    uint32_t arch_fsm_int_info_31_00; // 0x550
    uint32_t arch_fsm_int_info_63_32; // 0x554
    uint32_t __RESERVED_0x0558_0x0DFC_[554];
    uint32_t integ_cfg; // 0xE00
    uint32_t __RESERVED_0x0E04_0x0E04_[1];
    uint32_t integ_outputs; // 0xE08
    uint32_t __RESERVED_0x0E0C_0x100C_[129];
    uint32_t address_control_now; // 0x1010
    uint32_t decode_control_now; // 0x1014
    uint32_t __RESERVED_0x1018_0x1018_[1];
    uint32_t address_map_now; // 0x101C
    uint32_t low_power_control_now; // 0x1020
    uint32_t __RESERVED_0x1024_0x1024_[1];
    uint32_t turnaround_control_now; // 0x1028
    uint32_t hit_turnaround_control_now; // 0x102C
    uint32_t qos_class_control_now; // 0x1030
    uint32_t escalation_control_now; // 0x1034
    uint32_t qv_control_31_00_now; // 0x1038
    uint32_t qv_control_63_32_now; // 0x103C
    uint32_t rt_control_31_00_now; // 0x1040
    uint32_t rt_control_63_32_now; // 0x1044
    uint32_t timeout_control_now; // 0x1048
    uint32_t credit_control_now; // 0x104C
    uint32_t write_priority_control_31_00_now; // 0x1050
    uint32_t write_priority_control_63_32_now; // 0x1054
    uint32_t __RESERVED_0x1058_0x105C_[2];
    uint32_t queue_threshold_control_31_00_now; // 0x1060
    uint32_t queue_threshold_control_63_32_now; // 0x1064
    uint32_t __RESERVED_0x1068_0x1074_[4];
    uint32_t memory_address_max_31_00_now; // 0x1078
    uint32_t memory_address_max_43_32_now; // 0x107C
    uint32_t access_address_min0_31_00_now; // 0x1080
    uint32_t access_address_min0_43_32_now; // 0x1084
    uint32_t access_address_max0_31_00_now; // 0x1088
    uint32_t access_address_max0_43_32_now; // 0x108C
    uint32_t access_address_min1_31_00_now; // 0x1090
    uint32_t access_address_min1_43_32_now; // 0x1094
    uint32_t access_address_max1_31_00_now; // 0x1098
    uint32_t access_address_max1_43_32_now; // 0x109C
    uint32_t access_address_min2_31_00_now; // 0x10A0
    uint32_t access_address_min2_43_32_now; // 0x10A4
    uint32_t access_address_max2_31_00_now; // 0x10A8
    uint32_t access_address_max2_43_32_now; // 0x10AC
    uint32_t access_address_min3_31_00_now; // 0x10B0
    uint32_t access_address_min3_43_32_now; // 0x10B4
    uint32_t access_address_max3_31_00_now; // 0x10B8
    uint32_t access_address_max3_43_32_now; // 0x10BC
    uint32_t access_address_min4_31_00_now; // 0x10C0
    uint32_t access_address_min4_43_32_now; // 0x10C4
    uint32_t access_address_max4_31_00_now; // 0x10C8
    uint32_t access_address_max4_43_32_now; // 0x10CC
    uint32_t access_address_min5_31_00_now; // 0x10D0
    uint32_t access_address_min5_43_32_now; // 0x10D4
    uint32_t access_address_max5_31_00_now; // 0x10D8
    uint32_t access_address_max5_43_32_now; // 0x10DC
    uint32_t access_address_min6_31_00_now; // 0x10E0
    uint32_t access_address_min6_43_32_now; // 0x10E4
    uint32_t access_address_max6_31_00_now; // 0x10E8
    uint32_t access_address_max6_43_32_now; // 0x10EC
    uint32_t access_address_min7_31_00_now; // 0x10F0
    uint32_t access_address_min7_43_32_now; // 0x10F4
    uint32_t access_address_max7_31_00_now; // 0x10F8
    uint32_t access_address_max7_43_32_now; // 0x10FC
    uint32_t __RESERVED_0x1100_0x110C_[4];
    uint32_t dci_replay_type_now; // 0x1110
    uint32_t __RESERVED_0x1114_0x111C_[3];
    uint32_t refresh_control_now; // 0x1120
    uint32_t __RESERVED_0x1124_0x1124_[1];
    uint32_t memory_type_now; // 0x1128
    uint32_t __RESERVED_0x112C_0x116C_[17];
    uint32_t scrub_control0_now; // 0x1170
    uint32_t scrub_address_min0_now; // 0x1174
    uint32_t scrub_address_max0_now; // 0x1178
    uint32_t __RESERVED_0x117C_0x117C_[1];
    uint32_t scrub_control1_now; // 0x1180
    uint32_t scrub_address_min1_now; // 0x1184
    uint32_t scrub_address_max1_now; // 0x1188
    uint32_t __RESERVED_0x118C_0x118C_[1];
    uint32_t scrub_control2_now; // 0x1190
    uint32_t scrub_address_min2_now; // 0x1194
    uint32_t scrub_address_max2_now; // 0x1198
    uint32_t __RESERVED_0x119C_0x119C_[1];
    uint32_t scrub_control3_now; // 0x11A0
    uint32_t scrub_address_min3_now; // 0x11A4
    uint32_t scrub_address_max3_now; // 0x11A8
    uint32_t __RESERVED_0x11AC_0x11AC_[1];
    uint32_t scrub_control4_now; // 0x11B0
    uint32_t scrub_address_min4_now; // 0x11B4
    uint32_t scrub_address_max4_now; // 0x11B8
    uint32_t __RESERVED_0x11BC_0x11BC_[1];
    uint32_t scrub_control5_now; // 0x11C0
    uint32_t scrub_address_min5_now; // 0x11C4
    uint32_t scrub_address_max5_now; // 0x11C8
    uint32_t __RESERVED_0x11CC_0x11CC_[1];
    uint32_t scrub_control6_now; // 0x11D0
    uint32_t scrub_address_min6_now; // 0x11D4
    uint32_t scrub_address_max6_now; // 0x11D8
    uint32_t __RESERVED_0x11DC_0x11DC_[1];
    uint32_t scrub_control7_now; // 0x11E0
    uint32_t scrub_address_min7_now; // 0x11E4
    uint32_t scrub_address_max7_now; // 0x11E8
    uint32_t __RESERVED_0x11EC_0x11EC_[1];
    uint32_t feature_control_now; // 0x11F0
    uint32_t mux_control_now; // 0x11F4
    uint32_t rank_remap_control_now; // 0x11F8
    uint32_t scrub_control_now; // 0x11FC
    uint32_t t_refi_now; // 0x1200
    uint32_t t_rfc_now; // 0x1204
    uint32_t t_mrr_now; // 0x1208
    uint32_t t_mrw_now; // 0x120C
    uint32_t t_rdpden_now; // 0x1210
    uint32_t __RESERVED_0x1214_0x1214_[1];
    uint32_t t_rcd_now; // 0x1218
    uint32_t t_ras_now; // 0x121C
    uint32_t t_rp_now; // 0x1220
    uint32_t t_rpall_now; // 0x1224
    uint32_t t_rrd_now; // 0x1228
    uint32_t t_act_window_now; // 0x122C
    uint32_t __RESERVED_0x1230_0x1230_[1];
    uint32_t t_rtr_now; // 0x1234
    uint32_t t_rtw_now; // 0x1238
    uint32_t t_rtp_now; // 0x123C
    uint32_t __RESERVED_0x1240_0x1240_[1];
    uint32_t t_wr_now; // 0x1244
    uint32_t t_wtr_now; // 0x1248
    uint32_t t_wtw_now; // 0x124C
    uint32_t __RESERVED_0x1250_0x1250_[1];
    uint32_t t_xmpd_now; // 0x1254
    uint32_t t_ep_now; // 0x1258
    uint32_t t_xp_now; // 0x125C
    uint32_t t_esr_now; // 0x1260
    uint32_t t_xsr_now; // 0x1264
    uint32_t t_esrck_now; // 0x1268
    uint32_t t_ckxsr_now; // 0x126C
    uint32_t t_cmd_now; // 0x1270
    uint32_t t_parity_now; // 0x1274
    uint32_t t_zqcs_now; // 0x1278
    uint32_t t_rw_odt_clr_now; // 0x127C
    uint32_t __RESERVED_0x1280_0x12FC_[32];
    uint32_t t_rddata_en_now; // 0x1300
    uint32_t t_phyrdlat_now; // 0x1304
    uint32_t t_phywrlat_now; // 0x1308
    uint32_t __RESERVED_0x130C_0x130C_[1];
    uint32_t rdlvl_control_now; // 0x1310
    uint32_t rdlvl_mrs_now; // 0x1314
    uint32_t t_rdlvl_en_now; // 0x1318
    uint32_t t_rdlvl_rr_now; // 0x131C
    uint32_t wrlvl_control_now; // 0x1320
    uint32_t wrlvl_mrs_now; // 0x1324
    uint32_t t_wrlvl_en_now; // 0x1328
    uint32_t t_wrlvl_ww_now; // 0x132C
    uint32_t __RESERVED_0x1330_0x1344_[6];
    uint32_t phy_power_control_now; // 0x1348
    uint32_t t_lpresp_now; // 0x134C
    uint32_t phy_update_control_now; // 0x1350
    uint32_t __RESERVED_0x1354_0x1354_[1];
    uint32_t odt_timing_now; // 0x1358
    uint32_t t_odth_now; // 0x135C
    uint32_t odt_wr_control_31_00_now; // 0x1360
    uint32_t odt_wr_control_63_32_now; // 0x1364
    uint32_t odt_rd_control_31_00_now; // 0x1368
    uint32_t odt_rd_control_63_32_now; // 0x136C
    uint32_t __RESERVED_0x1370_0x137C_[4];
    uint32_t dq_map_control_15_00_now; // 0x1380
    uint32_t dq_map_control_31_16_now; // 0x1384
    uint32_t dq_map_control_47_32_now; // 0x1388
    uint32_t dq_map_control_63_48_now; // 0x138C
    uint32_t dq_map_control_71_64_now; // 0x1390
    uint32_t __RESERVED_0x1394_0x13AC_[7];
    uint32_t odt_cp_control_31_00_now; // 0x13B0
    uint32_t odt_cp_control_63_32_now; // 0x13B4
    uint32_t __RESERVED_0x13B8_0x1404_[20];
    uint32_t user_config0_now; // 0x1408
    uint32_t user_config1_now; // 0x140C
    uint32_t __RESERVED_0x1410_0x1FCC_[752];
    uint32_t periph_id_4; // 0x1FD0
    uint32_t __RESERVED_0x1FD4_0x1FDC_[3];
    uint32_t periph_id_0; // 0x1FE0
    uint32_t periph_id_1; // 0x1FE4
    uint32_t periph_id_2; // 0x1FE8
    uint32_t periph_id_3; // 0x1FEC
    uint32_t component_id_0; // 0x1FF0
    uint32_t component_id_1; // 0x1FF4
    uint32_t component_id_2; // 0x1FF8
    uint32_t component_id_3; // 0x1FFC
} REG_ST_DMC520;

#endif /* INTERNAL_REG_DMC520_H */
