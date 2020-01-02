/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <synquacer_debug.h>
#include <synquacer_mmap.h>
#include <low_level_access.h>

#include <internal/transaction_sw.h>

#define STATUS_ADDR 4

void set_transactionsw_off(
    uint32_t transactionsw_reg_addr,
    uint32_t disable_bit)
{
    unsigned int intsts;
    uint32_t value;

    SYNQUACER_DEV_LOG_DEBUG("  traSW disable_bit =  %08x\n", disable_bit);

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

    SYNQUACER_DEV_LOG_DEBUG("  traSW enable_bit =  %08x\n", enable_bit);

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
