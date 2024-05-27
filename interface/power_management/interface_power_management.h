/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERFACE_POWER_MANAGEMENT_H
#define INTERFACE_POWER_MANAGEMENT_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupInterfaces Interfaces
 * @{
 */

/*!
 * \defgroup GroupPowerManagement Power Management Modules interface
 *
 * \brief Interface definition for Power Management modules.
 *
 * \details This provides a generic interface for the Power Management
 *          modules.
 * @{
 */

/*!
 * \brief Power Management interface
 */
struct interface_power_management_api {
    /*!
     * \brief Get the power limit
     * \param id Identifier of the entity to collect it's power limit.
     * \param[out] power_limit The power limit of the given entity.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered.
     *      - The`id` not valid id.
     *      - The `power_limit` parameter is a null pointer.
     * \retval ::FWK_E_BUSY Limit for the requested id is not ready yet.
     */
    int (*get_limit)(fwk_id_t id, uint32_t *power_limit);
    /*!
     * \brief Set the power limit for an entity to be registered. This doesn't
     *        necessarily mean the power limit is set physically.
     * \param id Identifier of the entity to set the limit to.
     * \param power_limit The power limit to be set.
     *
     * \retval ::FWK_SUCCESS The power limit is set successfully
     * \retval One of the standard framework status codes.
     */
    int (*set_limit)(fwk_id_t id, uint32_t power_limit);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* INTERFACE_POWER_MANAGEMENT_H */
