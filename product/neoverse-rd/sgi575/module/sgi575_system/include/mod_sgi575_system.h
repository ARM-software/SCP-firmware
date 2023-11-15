/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGI575 System Support
 */

#ifndef MOD_SGI575_SYSTEM_H
#define MOD_SGI575_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupSGI575Module SGI575 Product Modules
 * \{
 */

/*!
 * \defgroup GroupSGI575System SGI575 System Support
 * \{
 */

/*!
 * \brief Additional SGI575 system power states.
 */
enum mod_sgi575_system_power_states {
    MOD_SGI575_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_SGI575_SYSTEM_POWER_STATE_SLEEP1,
    MOD_SGI575_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_sgi575_system_power_state_masks {
    MOD_SGI575_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_SGI575_SYSTEM_POWER_STATE_SLEEP0),
    MOD_SGI575_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_SGI575_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_sgi575_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_SGI575_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_SGI575_SYSTEM_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SGI575_SYSTEM_H */
