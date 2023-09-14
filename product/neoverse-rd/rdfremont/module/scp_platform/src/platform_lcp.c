/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Platform Support - LCP interface
 */

#include "platform_core.h"
#include "scp_css_mmap.h"

#include <internal/scp_platform.h>

#include <fwk_macros.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * UART_EN bit. Enables access to the LCP UART and routing of the LCP UART
 * interrupt.
 */
#define LCP_UART_EN_POS 0
#define LCP_UART_EN_VAL 0x1

/*
 * CPUWAIT bit in the system control register block. CPU boot wait control.
 */
#define LCP_CPUWAIT_POS 0
#define LCP_CPUWAIT_VAL 0x1

static bool is_lcp_idx_valid(uint8_t lcp_idx)
{
    uint8_t lcp_count;

    lcp_count = platform_get_core_count();

    if (lcp_idx < lcp_count) {
        return true;
    }

    return false;
}

static int enable_lcp_uart(uint8_t lcp_idx)
{
    FWK_RW uint32_t *lcp_uart_ctrl_reg;

    if (!is_lcp_idx_valid(lcp_idx)) {
        return FWK_E_RANGE;
    }

    lcp_uart_ctrl_reg = (FWK_RW uint32_t *)LCP_UART_CONTROL(lcp_idx);

    /* Enable access to the LCP UART and the LCP UART interrupt routing */
    *lcp_uart_ctrl_reg |= (LCP_UART_EN_VAL << LCP_UART_EN_POS);

    return FWK_SUCCESS;
}

static void release_lcp(uint8_t lcp_idx)
{
    FWK_RW uint32_t *cpu_wait_reg;

    cpu_wait_reg = (FWK_RW uint32_t *)LCP_SYS_CTRL_CPU_WAIT(lcp_idx);

    /* Deassert CPUWAIT to start LCP execution */
    *cpu_wait_reg &= ~(LCP_CPUWAIT_VAL << LCP_CPUWAIT_POS);
}

int platform_setup_lcp(void)
{
    uint8_t lcp_idx;
    int status;

    lcp_idx = 0;

    /*
     * Enable UART access for LCP0 only. If all the LCPs are allowed to access
     * the UART at the same time, the output will be unreadable. Hence,
     * restrict the LCP UART to single LCP for now.
     */
    status = enable_lcp_uart(lcp_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Release all the LCPs */
    for (lcp_idx = 0; lcp_idx < platform_get_core_count(); lcp_idx++) {
        release_lcp(lcp_idx);
    }

    return FWK_SUCCESS;
}
