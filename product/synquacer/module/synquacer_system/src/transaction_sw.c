/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "synquacer_common.h"

#include <internal/transaction_sw.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>

#include <inttypes.h>
#include <stdint.h>

#define STATUS_ADDR 4

void set_transactionsw_off(
    uint32_t transactionsw_reg_addr,
    uint32_t disable_bit)
{
    unsigned int intsts;
    uint32_t value;

    FWK_LOG_INFO("  traSW disable_bit =  %08" PRIx32, disable_bit);

    DI(intsts);

    /* read transactionsw */
    value = readl(transactionsw_reg_addr);

    /* Clear transationsw disable bit */
    value &= (~disable_bit);

    /* transation sw enable */
    writel(transactionsw_reg_addr, value);

    EI(intsts);

    /* setting wait */
    while ((readl(transactionsw_reg_addr + STATUS_ADDR) & disable_bit) != 0)
        continue;
}

void set_transactionsw_on(uint32_t transactionsw_reg_addr, uint32_t enable_bit)
{
    unsigned int intsts;
    uint32_t value;

    FWK_LOG_INFO("  traSW enable_bit =  %08" PRIx32, enable_bit);

    DI(intsts);

    /* read transactionsw */
    value = readl(transactionsw_reg_addr);

    /* Clear transactionsw disable bit */
    value |= (enable_bit);

    /* transaction sw enable */
    writel(transactionsw_reg_addr, value);

    EI(intsts);

    /* setting wait */
    while ((readl(transactionsw_reg_addr + STATUS_ADDR) & enable_bit) !=
           enable_bit) {
        continue;
    }
}
