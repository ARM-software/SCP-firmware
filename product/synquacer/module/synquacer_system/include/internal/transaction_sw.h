/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_TRANSACTION_SW_H
#define INTERNAL_TRANSACTION_SW_H

#include <stdint.h>

void set_transactionsw_off(
    uint32_t transactionsw_reg_addr,
    uint32_t disable_bit);

void set_transactionsw_on(uint32_t transactionsw_reg_addr, uint32_t enable_bit);

#endif /* INTERNAL_TRANSACTION_SW_H */
