/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     TC1 System Support
 */

#ifndef MOD_TC1_SYSTEM_H
#define MOD_TC1_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupTC1Module TC1 Product Modules
 * \{
 */

/*!
 * \defgroup GroupTC1System TC1 System Support
 * \{
 */

/*!
 * \brief Additional TC1 system power states.
 */
enum mod_tc1_system_power_states {
    MOD_TC1_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_TC1_SYSTEM_POWER_STATE_SLEEP1,
    MOD_TC1_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_tc1_system_power_state_masks {
    MOD_TC1_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_TC1_SYSTEM_POWER_STATE_SLEEP0),
    MOD_TC1_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_TC1_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_tc1_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_TC1_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_TC1_SYSTEM_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_TC1_SYSTEM_H */
