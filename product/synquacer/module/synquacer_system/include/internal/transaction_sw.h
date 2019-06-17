/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRANSACTION_SW_H
#define TRANSACTION_SW_H

void set_transactionsw_off(
    uint32_t transactionsw_reg_addr,
    uint32_t disable_bit);

void set_transactionsw_on(uint32_t transactionsw_reg_addr, uint32_t enable_bit);

#endif /* TRANSACTION_SW_H */
