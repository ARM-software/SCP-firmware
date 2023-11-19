/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Platform System Support
 */

#ifndef MOD_PLATFORM_SYSTEM_H
#define MOD_PLATFORM_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupPlatformModule Product Modules
 * \{
 */

/*!
 * \defgroup GroupPlatformSystem Platform System Support
 * \{
 */

/*!
 * \brief Additional Platform system power states.
 */
enum mod_platform_system_power_states {
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP1,
    MOD_PLATFORM_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_platform_system_power_state_masks {
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP0),
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_platform_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_PLATFORM_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_PLATFORM_SYSTEM_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PLATFORM_SYSTEM_H */
