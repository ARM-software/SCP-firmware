/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     TC0 System Support
 */

#ifndef MOD_TC0_SYSTEM_H
#define MOD_TC0_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupTC0Module TC0 Product Modules
 * \{
 */

/*!
 * \defgroup GroupTC0System TC0 System Support
 * \{
 */

/*!
 * \brief Additional TC0 system power states.
 */
enum mod_tc0_system_power_states {
    MOD_TC0_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_TC0_SYSTEM_POWER_STATE_SLEEP1,
    MOD_TC0_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_tc0_system_power_state_masks {
    MOD_TC0_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_TC0_SYSTEM_POWER_STATE_SLEEP0),
    MOD_TC0_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_TC0_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_tc0_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_TC0_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_TC0_SYSTEM_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_TC0_SYSTEM_H */
