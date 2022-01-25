/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_DT_BINDINGS_H
#define FWK_DT_BINDINGS_H

/* 
 * Auto generated module indexes for Device tree inclusion.
 * Match indices in main C code.
 */
#include "fwk_dt_bindings_gen.h"

/* Additional module bindings for DTS use */

/* 
 * DT bindings for common modules.
 * 
 * enums from framework headers converted to 
 * #defines for use in the .dts files
 */

/* -- mod_power_domain.h -- */

    /* enum mod_pd_type */
#define MOD_PD_TYPE_CORE            0
#define MOD_PD_TYPE_CLUSTER         1
#define MOD_PD_TYPE_DEVICE          2
#define MOD_PD_TYPE_DEVICE_DEBUG    3
#define MOD_PD_TYPE_SYSTEM          4
#define MOD_PD_TYPE_COUNT           5

    /* enum mod_pd_level */
#define MOD_PD_LEVEL_0      0
#define MOD_PD_LEVEL_1      1
#define MOD_PD_LEVEL_2      2
#define MOD_PD_LEVEL_3      3
#define MOD_PD_LEVEL_COUNT  4


    /* enum mod_pd_system_shutdown */
#define MOD_PD_SYSTEM_SHUTDOWN          0
#define MOD_PD_SYSTEM_COLD_RESET        1
#define MOD_PD_SYSTEM_WARM_RESET        2
#define MOD_PD_SYSTEM_SUB_SYSTEM_RESET  3
#define MOD_PD_SYSTEM_FORCED_SHUTDOWN   4  
#define MOD_PD_SYSTEM_COUNT             5

    /* enum mod_pd_state */
#define MOD_PD_STATE_OFF    0
#define MOD_PD_STATE_ON     1
#define MOD_PD_STATE_SLEEP  2
#define MOD_PD_STATE_COUNT  3

    /* enum mod_pd_state_mask */
#define MOD_PD_STATE_OFF_MASK   (1 << MOD_PD_STATE_OFF)
#define MOD_PD_STATE_ON_MASK    (1 << MOD_PD_STATE_ON)
#define MOD_PD_STATE_SLEEP_MASK (1 << MOD_PD_STATE_SLEEP)

/* ---- */

/* -- mod_system_power.h -- */
    /* enum mod_system_power_power_states */
#define MOD_SYSTEM_POWER_POWER_STATE_SLEEP0 (MOD_PD_STATE_COUNT)
#define MOD_SYSTEM_POWER_POWER_STATE_SLEEP1 (MOD_PD_STATE_COUNT + 1)
#define MOD_SYSTEM_POWER_POWER_STATE_COUNT  (MOD_PD_STATE_COUNT + 2)


    /* enum mod_system_power_api_idx */
#define MOD_SYSTEM_POWER_API_IDX_PD_DRIVER          0
#define MOD_SYSTEM_POWER_API_IDX_PD_DRIVER_INPUT    1
#define MOD_SYSTEM_POWER_API_COUNT                  2

/* ---- */

#endif /* FWK_DT_BINDINGS_H */
