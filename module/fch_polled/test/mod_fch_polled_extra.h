/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_FCH_POLLED_EXTRA_H
#define MOD_FCH_POLLED_EXTRA_H

#include <mod_timer.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

int fch_polled_extra_alarm_start(
    fwk_id_t alarm_id,
    unsigned int milliseconds,
    enum mod_timer_alarm_type type,
    void (*callback)(uintptr_t param),
    uintptr_t param);

int fch_polled_extra_alarm_stop(fwk_id_t alarm_id);

#endif /* MOD_FCH_POLLED_EXTRA_H */
