/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     TC2 Power Model (Thermal Management Driver)
 */

#ifndef MOD_TC2_POWER_MODEL_H
#define MOD_TC2_POWER_MODEL_H

//#include <mod_power_domain.h>

/*!
 * \addtogroup GroupTC2Module TC2 Product Modules
 * \{
 */

/*!
 * \defgroup GroupTC2PowerModel TC2 Power Model
 * \{
 */

/*!
 * \brief TC2 Power Model element configuration
 */
struct mod_tc2_power_model_dev_config {
    /*!
     * \brief Coefficient to compute the power from level and viceversa.
     *
     * \details This is an oversimplified example of element configuration. For
     *      the purpose of code reference, we assume that power and level are
     *      tied each other by a simple coefficient.
     */
    unsigned int coeff;
};

/*!
 * \brief Number of defined APIs for Power Model.
 */
enum mod_tc2_power_model_api_idx {
    /*! API index for the driver interface of the Thermal Management module */
    MOD_TC2_POWER_MODEL_THERMAL_DRIVER_API_IDX,

    /*! Number of exposed interfaces */
    MOD_TC2_POWER_MODEL_API_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_TC2_POWER_MODEL_H */
