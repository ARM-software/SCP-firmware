/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DIMM_SPD_H
#define DIMM_SPD_H

#include <mod_log.h>
#include <mod_n1sdp_dmc620.h>
#include <mod_n1sdp_i2c.h>

#include <stdint.h>

/*
 * DMC-620 ADDRESS_CONTROL register bit field values
 */
#define COL_BITS_NEXT_MASK      UINT32_C(0x00000003)
#define COL_BITS_NEXT_10_BITS   UINT32_C(0x00000002)
#define COL_BITS_NEXT_11_BITS   UINT32_C(0x00000003)
#define COL_BITS_NEXT_12_BITS   UINT32_C(0x00000004)

#define ROW_BITS_NEXT_MASK      UINT32_C(0x00000700)
#define ROW_BITS_NEXT_12_BITS   UINT32_C(0x00000100)
#define ROW_BITS_NEXT_13_BITS   UINT32_C(0x00000200)
#define ROW_BITS_NEXT_14_BITS   UINT32_C(0x00000300)
#define ROW_BITS_NEXT_15_BITS   UINT32_C(0x00000400)
#define ROW_BITS_NEXT_16_BITS   UINT32_C(0x00000500)
#define ROW_BITS_NEXT_17_BITS   UINT32_C(0x00000600)
#define ROW_BITS_NEXT_18_BITS   UINT32_C(0x00000700)

#define CID_BITS_NEXT_MASK      UINT32_C(0x60000000)
#define CID_BITS_NEXT_0_CID     UINT32_C(0x00000000)
#define CID_BITS_NEXT_1_CID     UINT32_C(0x20000000)
#define CID_BITS_NEXT_2_CID     UINT32_C(0x40000000)
#define CID_BITS_NEXT_3_CID     UINT32_C(0x60000000)

#define BANK_BITS_NEXT_MASK     UINT32_C(0x00070000)
#define BANK_BITS_NEXT_8_BANKS  UINT32_C(0x00030000)
#define BANK_BITS_NEXT_16_BANKS UINT32_C(0x00040000)

#define RANK_BITS_NEXT_MASK     UINT32_C(0x03000000)
#define RANK_BITS_NEXT_0        UINT32_C(0x00000000)
#define RANK_BITS_NEXT_1        UINT32_C(0x01000000)
#define RANK_BITS_NEXT_2        UINT32_C(0x02000000)
#define RANK_BITS_NEXT_3        UINT32_C(0x03000000)

/*
 * DMC-620 FORMAT_CONTROL register bit field values
 */
#define MEM_WIDTH_MASK          UINT32_C(0x00000003)
#define MEM_WIDTH_64            UINT32_C(0x00000002)
#define MEM_WIDTH_128           UINT32_C(0x00000003)

/*
 * DMC-620 MEMORY_TYPE register bit field values
 */
#define DMC620_MEM_TYPE_MASK       UINT32_C(0x00000003)
#define DMC620_MEM_TYPE_DDR4       UINT32_C(0x00000002)

#define MEM_BANK_GROUPS_NEXT_MASK  UINT32_C(0x00030000)
#define MEM_BANK_GROUPS_NEXT_0     UINT32_C(0x00000000)
#define MEM_BANK_GROUPS_NEXT_2     UINT32_C(0x00010000)
#define MEM_BANK_GROUPS_NEXT_4     UINT32_C(0x00030000)

#define MEM_DEV_WIDTH_NEXT_MASK    UINT32_C(0x00000300)
#define MEM_DEV_WIDTH_NEXT_4       UINT32_C(0x00000000)
#define MEM_DEV_WIDTH_NEXT_8       UINT32_C(0x00000100)
#define MEM_DEV_WIDTH_NEXT_16      UINT32_C(0x00000200)

#define T_REFI_NEXT_MASK           UINT32_C(0x00000FFF)
#define T_RFC_NEXT_MASK            UINT32_C(0x000003FF)
#define T_RFCFG_NEXT_MASK          UINT32_C(0x000FFC00)
#define T_RFCFC_NEXT_MASK          UINT32_C(0x0FF00000)
#define T_RCD_NEXT_MASK            UINT32_C(0x000001FF)
#define T_RAS_NEXT_MASK            UINT32_C(0x000003FF)
#define T_RP_NEXT_MASK             UINT32_C(0x000001FF)
#define T_WTW_L_NEXT_MASK          UINT32_C(0x00003F00)
#define T_WTW_S_NEXT_MASK          UINT32_C(0x0000003F)
#define T_WTW_DLR_NEXT_MASK        UINT32_C(0xF6000000)
#define T_RRD_S_NEXT_MASK          UINT32_C(0x0000000F)
#define T_RRD_L_NEXT_MASK          UINT32_C(0x00000F00)
#define T_RRD_DLR_NEXT_MASK        UINT32_C(0x0F000000)
#define T_FAW_NEXT_MASK            UINT32_C(0x0000003F)
#define T_FAW_DLR_NEXT_MASK        UINT32_C(0x00003F00)
#define T_MAWI_NEXT_MASK           UINT32_C(0x03FF0000)
#define T_WTR_S_NEXT_MASK          UINT32_C(0x0000003F)
#define T_WTR_L_NEXT_MASK          UINT32_C(0x00003F00)
#define T_WTR_CS_NEXT_MASK         UINT32_C(0x003F0000)
#define T_WR_NEXT_MASK             UINT32_C(0x0000003F)

#define LWR_NBBL_MASK              0x0F
#define UPPR_NBBL_MASK             0xF0

/*
 * SPD definitions
 */

/* SPD DRAM Device Type supported */
#define SPD_DDR4_SDRAM            0x0C

/* Column Address Bits */
#define SPD_COL_ADDR_BITS_MASK    0x07
#define SPD_COL_ADDR_BITS_9       0x00
#define SPD_COL_ADDR_BITS_10      0x01
#define SPD_COL_ADDR_BITS_11      0x02
#define SPD_COL_ADDR_BITS_12      0x03

/* Row Address Bits */
#define SPD_ROW_ADDR_BITS_MASK    0x38
#define SPD_ROW_ADDR_BITS_12      0x00
#define SPD_ROW_ADDR_BITS_13      0x08
#define SPD_ROW_ADDR_BITS_14      0x10
#define SPD_ROW_ADDR_BITS_15      0x18
#define SPD_ROW_ADDR_BITS_16      0x20
#define SPD_ROW_ADDR_BITS_17      0x28
#define SPD_ROW_ADDR_BITS_18      0x30

/* Bank Group bits */
#define SPD_BANK_GROUP_BITS_MASK  0xC0
#define SPD_BANK_GROUP_BITS_0     0x00
#define SPD_BANK_GROUP_BITS_2     0x40
#define SPD_BANK_GROUP_BITS_4     0x80

/* Bank Address bits on each chip-select */
#define SPD_BANK_BITS_MASK        0x30
#define SPD_BANK_BITS_2           0x00
#define SPD_BANK_BITS_3           0x10

/* SDRAM die capacity definition */
#define SPD_SDRAM_DENSITY_MASK    0xF
#define SPD_SDRAM_DENSITY_POS     0

/* Number of Package Ranks per DIMM supported */
#define SPD_PKG_RANK_BITS_MASK    0x38
#define SPD_PKG_RANK_BITS_OFFSET  3
#define SPD_PKG_RANK1             0x00
#define SPD_PKG_RANK2             0x01
#define SPD_PKG_RANK4             0x03
#define SPD_PKG_RANK8             0x07

/* SDRAM Package Type bits - SDRAM Package Type */
#define SPD_PKG_TYPE_BITS_MASK    0x80
#define SPD_PKG_TYPE_BITS_0       0x00
#define SPD_PKG_TYPE_BITS_1       0x80

/* Die Count bits - SDRAM Package Type */
#define SPD_DIE_CNT_BITS_MASK     0x70
#define SPD_DIE_CNT_BITS_0        0x00
#define SPD_DIE_CNT_BITS_2        0x10
#define SPD_DIE_CNT_BITS_4        0x30
#define SPD_DIE_CNT_BITS_8        0x70

/* Signal Loading bits - SDRAM Package Type */
#define SPD_SIG_LOAD_BITS_MASK    0x03
#define SPD_SIG_LOAD_BITS_0       0x00
#define SPD_SIG_LOAD_BITS_1       0x03
#define SPD_SIG_LOAD_BITS_2       0x01

/* SDRAM Primary bus width */
#define SPD_PRI_BUS_WIDTH_BITS_MASK   0x07
#define SPD_PRI_BUS_WIDTH_BITS_POS    0
#define SPD_PRI_BUS_WIDTH_BITS_8      0x00
#define SPD_PRI_BUS_WIDTH_BITS_16     0x01
#define SPD_PRI_BUS_WIDTH_BITS_32     0x02
#define SPD_PRI_BUS_WIDTH_BITS_64     0x03

/* SDRAM Device width */
#define SDRAM_DEVICE_WIDTH_MASK   0x07
#define SDRAM_DEVICE_WIDTH_POS    0
#define SDRAM_DEVICE_WIDTH_0      0x00
#define SDRAM_DEVICE_WIDTH_1      0x01
#define SDRAM_DEVICE_WIDTH_2      0x02
#define SDRAM_DEVICE_WIDTH_3      0x03

#define SPD_T_MAC_MASK       0x0F
#define SPD_T_MAC_0          0x00
#define SPD_T_MAC_1          0x01
#define SPD_T_MAC_2          0x02
#define SPD_T_MAC_3          0x03
#define SPD_T_MAC_4          0x04
#define SPD_T_MAC_5          0x05
#define SPD_T_MAC_6          0x06
#define SPD_T_MAC_7          0x07
#define SPD_T_MAC_8          0x08

#define FTB                  1U
#define MTB                  125U
#define DIMM0_SPD_SLAVE      0x50
#define DIMM1_SPD_SLAVE      0x51
#define WRITE_PAGE0          0x36
#define WRITE_PAGE1          0x37

#define SPD_PAGE0_START      0
#define SPD_PAGE1_START      256
#define MAX_SPD_PAGE0        255
#define MAX_SPD_PAGE1        511

#define SPD_W_TRANSFER_SIZE  2
#define SPD_R_TRANSFER_SIZE  1
#define SPD_STOP             1


/*
 * SPD - Base Configuration and DRAM Parameter structure
 *  Block   0
 *  Bytes   0 - 127
 *  Address 0x000 - 0x07F
 *  This section defines parameters that are common to all DDR4 module types
 */
struct ddr4_dram_param {
    /* Number of Bytes used */
    uint8_t num_bytes;
    /* SPD Revision */
    uint8_t spd_rev;
    /* Key Byte / DRAM Device Type */
    uint8_t kb_dram_type;
    /* Key Byte / Module Type */
    uint8_t kb_mod_type;
    /* SDRAM Density and Banks */
    uint8_t sdram_density_banks;
    /* SDRAM Addressing */
    uint8_t sdram_addr;
    /* SDRAM Package Type */
    uint8_t sdram_pkg_type;
    /* SDRAM Optional Features */
    uint8_t sdram_opt_features;
    /* SDRAM Thermal and Refresh Options */
    uint8_t sdram_temp_refresh;
    /* Other SDRAM Optional Features */
    uint8_t sdram_other_features;
    /* Secondary SDRAM Package Type */
    uint8_t secdry_sdram_pck_type;
    /* Module Nominal Voltage, VDD */
    uint8_t mod_nom_volt;
    /* Module Organization */
    uint8_t mod_org;
    /* Module Memory Bus Width */
    uint8_t mod_mem_bus_width;
    /* Module Thermal Sensor */
    uint8_t mod_temp_sensor;
    /* Extended module type */
    uint8_t extnded_mod_type;
    /* Reserved */
    uint8_t reserved_byte16;
    /* Timebases */
    uint8_t timebases;
    /* SDRAM Minimum Cycle Time (tCKAVGmin) */
    uint8_t tckavgmin;
    /* SDRAM Maximum Cycle Time (C) */
    uint8_t tckavgmax;
    /* CAS Latencies Supported, First Byte */
    uint8_t cas_lat_sup_1;
    /* CAS Latencies Supported, Second Byte */
    uint8_t cas_lat_sup_2;
    /* CAS Latencies Supported, Third Byte */
    uint8_t cas_lat_sup_3;
    /* CAS Latencies Supported, Fourth Byte */
    uint8_t cas_lat_sup_4;
    /* Minimum CAS Latency Time (tAAmin) */
    uint8_t taamin;
    /* Minimum RAS to CAS Delay Time (tRCDmin)*/
    uint8_t trcdmin;
    /* Minimum Row Precharge Delay Time (tRPmin) */
    uint8_t trpmin;
    /* Upper Nibbles for tRASmin and tRCmin */
    uint8_t uppr_nbls_trasmin_trcmin;
    /* Minimum Active to Precharge Delay Time(tRASmin), LSB */
    uint8_t trasmin_lsb;
    /* Minimum Active to Active/Refresh Delay Time (tRCmin), LSB */
    uint8_t trcmin_lsb;
    /* Minimum Refresh Recovery Delay Time (tRFC1min), LSB */
    uint8_t trfc1min_lsb;
    /* Minimum Refresh Recovery Delay Time (tRFC1min), MSB */
    uint8_t trfc1min_msb;
    /* Minimum Refresh Recovery Delay Time (tRFC2min), LSB */
    uint8_t trfc2min_lsb;
    /* Minimum Refresh Recovery Delay Time (tRFC2min), MSB */
    uint8_t trfc2min_msb;
    /* Minimum Refresh Recovery Delay Time (tRFC4min), LSB */
    uint8_t trfc4min_lsb;
    /* Minimum Refresh Recovery Delay Time (tRFC4min), MSB */
    uint8_t trfc4min_msb;
    /* Minimum Four Activate Window Time(tFAWmin), Most Significant Nibble */
    uint8_t tfawmin_msn;
    /* Minimum Four Activate Window Delay Time (tFAWmin), LSB */
    uint8_t tfawmin_lsb;
    /* Minimum Activate to Activate Delay Time(tRRD_Smin), diff bank group */
    uint8_t trrd_smin;
    /* Minimum Activate to Activate Delay Time (tRRD_Lmin), same bank group */
    uint8_t trrd_lmin;
    /* Minimum CAS to CAS Delay Time (tCCD_Lmin), same bank group */
    uint8_t tccd_lmin;
    /* Upper Nibble for tWRmin */
    uint8_t twrmin_un;
    /* Minimum Write Recovery Time (tWRmin) */
    uint8_t twrmin;
    /* Upper Nibble for tWTRmin */
    uint8_t twtrmin_un;
    /* Minimum Write to Read Time (tWTR_Smin), different bank group */
    uint8_t twtr_smin;
    /* Minimum Write to Read Time (tWTR_Lmin), same bank group */
    uint8_t twtr_lmin;
    /* Reserved */
    uint8_t reserved_byte46_59[14];
    /* Connector to SDRAM Bit Mapping, Bytes 60 - 77*/
    uint8_t conn_sdram_bit_map_60_77[18];
    /* Reserved */
    uint8_t reserved_byte78_116[39];
    /* Fine Offset for Minimum CAS to CAS Delay Time (tCCD_Lmin), same bank */
    uint8_t tccd_lmin_fine;
    /*
     * Fine Offset for Minimum Activate to Activate Delay Time(tRRD_Lmin),
     * same bank
     */
    uint8_t trrd_lmin_fine;
    /*
     * Fine Offset for Minimum Activate to Activate Delay Time (tRRD_Smin),
     * different bank group
     */
    uint8_t trrd_smin_fine;
    /* Fine Offset for Minimum Active to Active/Refresh Delay Time (tRCmin) */
    uint8_t trcmin_fine;
    /* Fine Offset for Minimum Row Precharge Delay Time (tRPmin) */
    uint8_t trpmin_fine;
    /* Fine Offset for Minimum RAS to CAS Delay Time (tRCDmin) */
    uint8_t trcdmin_fine;
    /* Fine Offset for Minimum CAS Latency Time (tAAmin) */
    uint8_t taamin_fine;
    /* Fine Offset for SDRAM Maximum Cycle Time (tCKAVGmax) */
    uint8_t tckavgmax_fine;
    /* Fine Offset for SDRAM Minimum Cycle Time (tCKAVGmin) */
    uint8_t tckavgmin_fine;
    /* Cyclical Redundancy Code (CRC) for Base Configuration Section, LSB */
    uint8_t crc_lsb;
    /* Cyclical Redundancy Code (CRC) for Base Configuration Section, MSB */
    uint8_t crc_msb;
} __attribute__((packed));

struct ddr4_spd {
  struct ddr4_dram_param dram_param;
  uint8_t standard_mod[64];
  uint8_t hybrid_mod[64];
  uint8_t hybrid_ext_func[64];
  uint8_t mfg_info[64];
  uint8_t end_usr[128];
} __attribute__((packed));

/*
 * SPD API function prototypes
 */

/*
 * Brief - Function to initialize and check DIMM SPD values
 *
 * param - i2c_api - Pointer to I2C master APIs
 * param - ddr - Pointer to DIMM information structure
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the SPD data is wrong
 */
int dimm_spd_init_check(struct mod_n1sdp_i2c_master_api_polled *i2c_api,
                         struct dimm_info *ddr);

/*
 * Brief - Function to print the DIMM card information
 *
 * retval - NONE
 */
void dimm_spd_mem_info(void);

/*
 * Brief - Function to calculate DMC-620 ADDRESS_CONTROL register value
 *
 * param - value - Pointer to variable where calculated value is saved
 * param - info - Pointer to DIMM information structure
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_address_control(uint32_t *value, struct dimm_info *info);

/*
 * Brief - Function to calculate DMC-620 FORMAT_CONTROL register value
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_format_control(uint32_t *value);

/*
 * Brief - Function to calculate DMC-620 MEMORY_TYPE register value
 *
 * param - value - Pointer to variable where calculated value is saved
 * param - info - Pointer to DIMM information structure
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_memory_type(uint32_t *value, struct dimm_info *info);

/*
 * Brief - Function to calculate tREFI timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_refi(uint32_t *value);

/*
 * Brief - Function to calculate tRFC timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_rfc(uint32_t *value);

/*
 * Brief - Function to calculate tRCD timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_rcd(uint32_t *value);

/*
 * Brief - Function to calculate tRAS timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_ras(uint32_t *value);

/*
 * Brief - Function to calculate tRP timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_rp(uint32_t *value);

/*
 * Brief - Function to calculate tRRD timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_rrd(uint32_t *value);

/*
 * Brief - Function to calculate tWTR timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 * param - info - Pointer to DIMM information structure
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_wtr(uint32_t *value, struct dimm_info *info);

/*
 * Brief - Function to calculate tACT_WINDOW timing parameter
 *
 * param - value - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DATA - if the value cannot be calculated
 */
int dimm_spd_t_act_window(uint32_t *value);

/*
 * Brief - Function to calculate total DIMM size
 *
 * param - size_gb - Pointer to variable where calculated value is saved
 *
 * retval - FWK_SUCCESS - if the operation is succeeded
 *          FWK_E_DEVICE - if the SPD value is invalid
 */
int dimm_spd_calculate_dimm_size_gb(uint32_t *size_gb);

#endif /* DIMM_SPD_H */
