/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SYNQUACER_SYSTEM_H
#define MOD_SYNQUACER_SYSTEM_H

#include <mod_power_domain.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupSYNQUACERSystem SYNQUACER System Support
 *
 * \brief SynQuacer System module.
 *
 * \details This module implements a SynQuacer system driver
 *
 * @{
 */

/*!
 * \brief Additional SYNQUACER system power states.
 */
enum mod_synquacer_system_power_states {
    MOD_SYNQUACER_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_SYNQUACER_SYSTEM_POWER_STATE_SLEEP1,
    MOD_SYNQUACER_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_synquacer_system_power_state_masks {
    MOD_SYNQUACER_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_SYNQUACER_SYSTEM_POWER_STATE_SLEEP0),
    MOD_SYNQUACER_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_SYNQUACER_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_synquacer_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_SYNQUACER_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_SYNQUACER_SYSTEM_API_COUNT
};

/*!
 * \brief Module Context
 */
struct synquacer_system_ctx {
    /*! pointer to the power domain module. */
    const struct mod_pd_restricted_api *mod_pd_restricted_api;

    /*! pointer to the ccn512 module. */
    const struct mod_ccn512_api *ccn512_api;

    /*! pointer to the f_i2c module. */
    const struct mod_f_i2c_api *f_i2c_api;

    /*! pointer to the hsspi module. */
    const struct mod_hsspi_api *hsspi_api;
};

/*!
 * \brief SynQuacer System Module Context
 */
extern struct synquacer_system_ctx synquacer_system_ctx;

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SYNQUACER_SYSTEM_H */
