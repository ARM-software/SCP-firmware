/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power coordinator module header.
 */

#ifndef MOD_POWER_COORDINATOR_H
#define MOD_POWER_COORDINATOR_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupPowerCoordinator Power coordinator
 *
 * \details Responsible for coordinating the modules related to the power
 *     management.
 *
 * \{
 */

/*!
 * \brief Power coordinator interface.
 */
struct mod_power_coordinator_api {
    /*!
     * \brief Get the coordinator period.
     *
     * \param id Coordinator ID.
     * \param[out] period The coordinator period.
     *
     * \retval ::FWK_SUCCESS The coordinator period is returned successfully.
     */
    int (*get_coordinator_period)(fwk_id_t id, uint32_t *period);

    /*!
     * \brief Set the coordinator period.
     *
     * \param id Coordinator ID.
     * \param period The coordinator period.
     *
     * \retval ::FWK_SUCCESS The coordinator period is set successfully.
     */
    int (*set_coordinator_period)(fwk_id_t id, uint32_t period);
};

/*!
 * \brief API indices.
 */
enum mod_power_coordinator_api_idx {
    /*! Power Coordinator period. */
    MOD_POWER_COORDINATOR_API_IDX_PERIOD,

    /*! Number of defined APIs. */
    MOD_POWER_COORDINATOR_API_IDX_COUNT,
};

/*!
 * \brief Power coordinator notification indices.
 */
enum mod_power_coordinator_notification_idx {
    /*! Period changed notification. */
    MOD_POWER_COORDINATOR_NOTIFICATION_IDX_PERIOD_CHANGED,

    /*! Number of defined notifications. */
    MOD_POWER_COORDINATOR_NOTIFICATION_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_POWER_COORDINATOR_H */
