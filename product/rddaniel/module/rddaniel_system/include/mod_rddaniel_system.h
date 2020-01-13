/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RD-Daniel System Support
 */

#ifndef MOD_RDDANIEL_SYSTEM_H
#define MOD_RDDANIEL_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupRDDANIELModule RDDANIEL Product Modules
 * @{
 */

/*!
 * \defgroup GroupRDDANIELSystem RDDANIEL System Support
 * @{
 */

/*!
 * \brief Additional RDDANIEL system power states.
 */
enum mod_rddaniel_system_power_states {
    MOD_RDDANIEL_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_RDDANIEL_SYSTEM_POWER_STATE_SLEEP1,
    MOD_RDDANIEL_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_rddaniel_system_power_state_masks {
    MOD_RDDANIEL_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_RDDANIEL_SYSTEM_POWER_STATE_SLEEP0),
    MOD_RDDANIEL_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_RDDANIEL_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_rddaniel_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_RDDANIEL_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_RDDANIEL_SYSTEM_API_COUNT
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RDDANIEL_SYSTEM_H */
