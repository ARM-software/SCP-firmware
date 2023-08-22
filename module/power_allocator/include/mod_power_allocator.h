/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power allocator module header.
 */

#ifndef MOD_POWER_ALLOCATOR_H
#define MOD_POWER_ALLOCATOR_H

#include <fwk_id.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupPowerAllocator Power Allocator
 *
 * \details Responsible for power allocation and management in SCP
 *
 * \{
 */

/*!
 * \brief Power Allocator interface.
 */
struct mod_power_allocator_api {
    /*!
     * \brief Get the power cap imposed on a power capping domain.
     *
     * \param domain_id Power capping domain id.
     * \param[out] cap Power cap imposed on the domain specified by the
     *      domain id.
     *
     * \retval ::FWK_SUCCESS The cap is returned successfully.
     */
    int (*get_cap)(fwk_id_t domain_id, uint32_t *cap);

    /*!
     * \brief Set a power cap for a power capping domain.
     *
     * \param domain_id Power capping domain id.
     * \param cap The required power cap to be set for a domain specified by the
     *      domain id. Setting this value to zero means disabling power capping.
     *
     * \retval ::FWK_SUCCESS The cap is set successfully.
     * \retval ::FWK_PENDING The cap hasn't been set yet. The power allocator
     *      is processing the cap set request. Once the power allocator sets a
     *      new power cap, it will notify the registered modules about it.
     */
    int (*set_cap)(fwk_id_t domain_id, uint32_t cap);
};

/*!
 * \brief API indices.
 */
enum mod_power_allocator_api_idx {
    /*! Cap set and get API. */
    MOD_POWER_ALLOCATOR_API_IDX_CAP,

    /*! Number of defined APIs. */
    MOD_POWER_ALLOCATOR_API_IDX_COUNT,
};

/*!
 * \brief Power allocator notification indices.
 */
enum mod_power_allocator_notification_idx {
    /*! Power cap changed notification. */
    MOD_POWER_ALLOCATOR_NOTIFICATION_IDX_CAP_CHANGED,

    /*! Number of defined notifications. */
    MOD_POWER_ALLOCATOR_NOTIFICATION_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_POWER_ALLOCATOR_H */
