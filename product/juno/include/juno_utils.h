/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_UTILS_H
#define JUNO_UTILS_H

#include <stdbool.h>
#include <stdint.h>

void juno_utils_open_snoop_gate_and_wait(volatile uint32_t *snoop_ctrl);
void juno_utils_close_snoop_gate(volatile uint32_t *snoop_ctrl);

void juno_utils_atclk_clock_div_set(uint32_t divider);
bool juno_utils_atclk_clock_div_set_check(uint32_t divider);

void juno_utils_atclk_clock_sel_set(uint32_t source);
bool juno_utils_atclk_clock_sel_set_check(uint32_t source);

void juno_utils_traceclk_clock_div_set(uint32_t divider);
bool juno_utils_traceclk_clock_div_set_check(uint32_t divider);

void juno_utils_traceclk_clock_sel_set(uint32_t source);
bool juno_utils_traceclk_clock_sel_set_check(uint32_t source);

void juno_utils_pclkdbg_clock_div_set(uint32_t divider);
bool juno_utils_pclkdbg_clock_div_set_check(uint32_t divider);

void juno_utils_system_clock_enable(uint32_t mask);

void juno_utils_smc_init(void);

#endif /* JUNO_UTILS_H */
