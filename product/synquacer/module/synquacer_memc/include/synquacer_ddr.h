/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_DDR_H
#define SYNQUACER_DDR_H

#include "synquacer_mmap.h"

#include <stdbool.h>
#include <stdint.h>

#define DRAM_DDR4 0x00000002
#define DIMM_SLOT_NUM 4
#define SPD_BYTE_TOP_ADDR 0x00
#define SPD_READ_BYTE_NUM 128

#define MODULE_TYPE_RDIMM 0x01
#define MODULE_TYPE_UDIMM 0x02
#define MODULE_TYPE_72BITSOUDIMM 0x09

#define FW_MODULE_CAPACITY_16GB 0x4000
#define FW_MODULE_CAPACITY_8GB 0x2000
#define FW_MODULE_CAPACITY_4GB 0x1000

#define DDR_USE_CH0 0x1
#define DDR_USE_CH1 0x2
#define DDR_USE_DUAL_CH (DDR_USE_CH0 | DDR_USE_CH1)

#define RDIMM_16GBPERSLOT_1SLOTPERCH 0x1
#define RDIMM_16GBPERSLOT_2SLOTPERCH 0x2
#define UDIMM_4GBPERSLOT_1SLOTPERCH 0x3
#define SOUDIMM_72BIT_16GBPERSLOT_1SLOTPERCH 0x4
#define SOUDIMM_72BIT_8GBPERSLOT_1SLOTPERCH 0x5
#define UDIMM_8GBPERSLOT_1SLOTPERCH 0x6

#define BANK_BIT_NEXT 0x00040000

#define DRAM_AREA_1_START_ADDR UINT64_C(0x0080000000)

/* top 64MiB in DRAM1 region is reserved for Secure App */
#define DRAM_RESERVED_FOR_SECURE_APP_SIZE UINT64_C(0x04000000)
#define DRAM_AREA_1_END_ADDR UINT64_C(0x0100000000)
#define DRAM_AREA_2_START_ADDR UINT64_C(0x0880000000)
#define DRAM_AREA_2_END_ADDR UINT64_C(0x1000000000)
#define DRAM_AREA_3_START_ADDR UINT64_C(0x8800000000)
#define DRAM_AREA_3_END_ADDR UINT64_C(0x9000000000)

#define DRAM_AREA_1_SIZE (DRAM_AREA_1_END_ADDR - DRAM_AREA_1_START_ADDR)
#define DRAM_AREA_2_SIZE (DRAM_AREA_2_END_ADDR - DRAM_AREA_2_START_ADDR)
#define DRAM_AREA_3_SIZE (DRAM_AREA_3_END_ADDR - DRAM_AREA_3_START_ADDR)

#define SPD_DTIC_SPA0 (0x36) /* set page address to 0 */
#define SPD_DTIC_SPA1 (0x37) /* set page address to 1 */
#define SPD_PAGE_SIZE (256)
#define SPD_NUM_OF_PAGE (2)

#define SPD_STORE_AREA_OFFSET (0x0000F000)
#define SPD_STORE_AREA_SIZE (DIMM_SLOT_NUM * (SPD_PAGE_SIZE * SPD_NUM_OF_PAGE))
#define SPD_STORE_ADDR (NONTRUSTED_RAM_BASE + SPD_STORE_AREA_OFFSET)

void fw_ddr_init(void);
int fw_ddr_spd_param_check(void);
bool fw_get_ddr4_sdram_ecc_available(void);
uint8_t fw_get_used_memory_ch(void);
uint32_t fw_get_memory_type_next(void);
uint32_t fw_get_address_control_next(void);
uint32_t fw_get_ddr4_sdram_dq_map_control(uint8_t i);
uint32_t fw_get_ddr4_sdram_total_size(void);
uint32_t fw_get_memory_type(void);

#endif /* SYNQUACER_DDR_H */
