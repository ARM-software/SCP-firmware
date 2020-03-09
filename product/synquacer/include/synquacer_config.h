/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_CONFIG_H
#define SYNQUACER_CONFIG_H

#include "synquacer_mmap.h"

#include <stdbool.h>
#include <stdint.h>

#define THRESHOLD_TYPE_UNR (0)
#define THRESHOLD_TYPE_UCR (1)
#define THRESHOLD_TYPE_UNC (2)
#define THRESHOLD_TYPE_NUM_MAX (3)

#define SENSOR_NUM_MAX (256)

#define PARTITION_NUM_MAX (8 * 8)

#define check_valid_bit(bitmap, pos) (((bitmap) & (1 << (pos))) != 0)
#define dram_ecc_is_enabled() (eeprom_config.pec_ecc_status != 0)

struct unused_eeprom_config_head_part_t {
    uint8_t mac_addr[8];
    uint32_t hm_me_addr_high;
    uint32_t hm_me_addr_low;
    uint32_t hm_me_size;
    uint32_t mh_me_addr_high;
    uint32_t mh_me_addr_low;
    uint32_t mh_me_size;
    uint32_t packet_me_addr_low;
    uint32_t packet_me_size;
    uint32_t magic_word;
    uint8_t reserved_212[212];
    uint32_t reserved_1;
    uint32_t reserved_2;
};

#define EEPROM_CONFIG_T_START_OFFSET \
    sizeof(struct unused_eeprom_config_head_part_t)

struct nat_config {
    uint8_t valid;
    uint8_t reserved[3];
    uint8_t ext_ipaddr[4];
};

typedef struct {
    uint32_t system_composition;

    uint8_t reverved_1;
    uint8_t reserved_valid_sbb_after[3];
    uint8_t reserved_8[8];
    uint8_t reverved_1_1;
    uint8_t reverved_1_2;

    uint8_t reserved_2[2];

    uint8_t reverved_1_3;
    uint8_t reverved_1_4;
    uint8_t reverved_1_5;

    uint8_t reserved_1[1];

    uint64_t reserved_8_2;
    uint64_t reserved_8_3;
    uint8_t pcie_cache_snoop_valid[2];
    uint8_t reverved_1_6;

    uint8_t reserved_205[205];

    uint8_t reverved_1_7[8][8];

    uint8_t reserved_192[192];

    uint8_t reverved_1_8[8][8][8];

    uint8_t reverved_1_9;
    uint8_t reverved_1_10[7];
    uint8_t reverved_1_11[8];
    uint8_t reverved_1_12[8][8];

    uint8_t reserved_176[176];

    uint8_t reverved_1_13[PARTITION_NUM_MAX][8];

    uint8_t sbb_ecc_status;
    uint8_t pec_ecc_status;

    uint8_t reserved_before_sensor_threshold[0x1300 - 0x802];

    uint8_t sensor_thresh[THRESHOLD_TYPE_NUM_MAX][SENSOR_NUM_MAX];
} eeprom_config_t;

extern eeprom_config_t eeprom_config;

#endif /* SYNQUACER_CONFIG_H */
