/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LCP_DEVICE_H
#define LCP_DEVICE_H

#include <lcp_mmap.h>

#include <fmw_cmsis_lcp.h>

#include <stdint.h>

#define CTRL_ENABLE_POS      0U
#define CTRL_EXTR_EN_POS     1U
#define CTRL_EXTR_IS_CLK_POS 2U
#define CTRL_INT_EN_POS      3U

#define CTRL_ENABLE_MASK      (1UL << CTRL_ENABLE_POS)
#define CTRL_EXTR_EN_POS_MASK (1UL << CTRL_EXTR_EN_POS)
#define CTRL_EXTR_IS_CLK_MASK (1UL << CTRL_EXTR_IS_CLK_POS)
#define CTRL_INT_EN_MASK      (1UL << CTRL_INT_EN_POS)

struct lcp_timer_reg_str {
    FWK_RW uint32_t CTRL;
    FWK_RW uint32_t VALUE;
    FWK_RW uint32_t RELOAD;
    FWK_RW uint32_t INTSTATUS;
};

#define LCP_TIMER_REG_S ((struct lcp_timer_reg_str *)LCP_TIMER_BASE_S)

#endif /* LCP_DEVICE_H */
