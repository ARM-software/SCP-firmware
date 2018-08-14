/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CLARK System Support
 */

#ifndef MOD_CLARK_SYSTEM_H
#define MOD_CLARK_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupCLARKModule CLARK Product Modules
 * @{
 */

/*!
 * \defgroup GroupCLARKSystem CLARK System Support
 * @{
 */

/*!
 * \brief Additional CLARK system power states.
 */
enum mod_clark_system_power_states {
    MOD_CLARK_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_CLARK_SYSTEM_POWER_STATE_SLEEP1,
    MOD_CLARK_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_clark_system_power_state_masks {
    MOD_CLARK_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_CLARK_SYSTEM_POWER_STATE_SLEEP0),
    MOD_CLARK_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_CLARK_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_clark_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_CLARK_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_CLARK_SYSTEM_API_COUNT
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CLARK_SYSTEM_H */
