/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_INIT_H
#define DDR_INIT_H

#include "synquacer_mmap.h"

#include <internal/reg_DDRPHY_CONFIG.h>
#include <internal/reg_DMC520.h>

#include <fwk_log.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>

#define printf(...) FWK_LOG_INFO("[DDR] " __VA_ARGS__)
#define pr_err(...) FWK_LOG_ERR("[DDR] " __VA_ARGS__)

#define dmb __DMB

#define REG_DDRPHY_CONFIG_0_BA UINT32_C(0x7F210000)
#define REG_DDRPHY_CONFIG_1_BA UINT32_C(0x7F610000)

#define REG_DMC520_0_BA UINT32_C(0x4E000000)
#define REG_DMC520_1_BA UINT32_C(0x4E100000)
#define REG_DMC520_3_BA UINT32_C(0x4E300000)

#define DDR_TRAINING_ON
#define DDR_DQSTRAINWA_ON
#define DDR_WAIT_TIMEOUT_US UINT32_C(1000000)

extern int ddr_dual_ch_init_mp(void);
extern int ddr_ch0_init_mp(void);
extern int ddr_ch1_init_mp(void);
extern uint8_t ddr_is_secure_dram_enabled(void);

#endif /*DDR_INIT_H */
