/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Reset domain HAL.
 */

#ifndef MOD_RESET_DOMAIN_H
#define MOD_RESET_DOMAIN_H

#include <fwk_id.h>
#include <fwk_module_idx.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupResetDomain Reset Domain HAL
 *
 * \details Support for setting the state of reset domains.
 *
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_reset_domain_api_type {
    /*!
     * \brief HAL API.
     *
     * \note This API identifier implements the mod_reset_domain_api interface.
     */
    MOD_RESET_DOMAIN_API_TYPE_HAL,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_RESET_DOMAIN_API_COUNT,
};

/*!
 * \brief Reset domain modes.
 */
enum mod_reset_domain_mode {
    /*!
     * \brief Indicates whether auto reset mode is supported by a reset domain.
     */
    MOD_RESET_DOMAIN_AUTO_RESET = (1UL << 0),

    /*!
     * \brief Indicates whether async auto reset mode is supported
     *     by a reset domain.
     */
    MOD_RESET_DOMAIN_MODE_AUTO_RESET_ASYNC = (1UL << 1),

    /*!
     * \brief Indicates whether explicit reset mode is supported by a reset
     *     domain
     */
    MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT = (1UL << 2),

    /*!
     * \brief Indicates whether explicit reset mode is supported by a reset
     *     domain
     */
    MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT = (1UL << 3),
};


/*!
 * \brief Reset domain capabilities.
 */
enum mod_reset_domain_capabilities {
    /*!
     * \brief Indicates whether notifications are supported by a reset domain.
     */
    MOD_RESET_DOMAIN_CAP_NOTIFICATION = (1UL << 0),

    /*!
     * \brief Indicates whether async reset is supported by a reset domain.
     */
    MOD_RESET_DOMAIN_CAP_ASYNC =  (1UL << 1)
};

/*!
 * \brief Reset domain module configuration data.
 *
 */
struct mod_reset_domain_config {
    /*!
     * \brief Identifier of the reset domain notification for auto reset status.
     *
     * \details Modules that are interested in the status of an auto reset of a
     *     reset domain will use below notification identifier to subscribe to
     *     the notification. For example, SCMI agents may request notification
     *     for the status of an auto reset operation on a reset domain.
     */
    fwk_id_t notification_id;
};

/*!
 * \brief Reset domain element configuration data.
 */
struct mod_reset_domain_dev_config {
    /*! Driver identifier */
    fwk_id_t driver_id;

    /*! Driver API identifier */
    fwk_id_t driver_api_id;

     /*! Supported modes, see mod_reset_domain_mode */
    enum mod_reset_domain_mode modes;

    /*! Supported capabilities, see mod_reset_domain_capabilities */
    enum mod_reset_domain_capabilities capabilities;

    /*! Maximum time (in microseconds) required for the reset to take effect */
    unsigned int latency;
};

/*!
 * \brief Reset domain HAL interface.
 *
 * \details The interface the reset domain clients relies on to perform
 *      actions on a reset domain.
 */
struct mod_reset_domain_api {
    /*!
     * \brief Change reset state of the domain \p element_id
     *
     * \param element_id Reset element identifier.
     * \param mode Reset domain mode.
     * \param reset_state Reset domain state as defined in SCMIv2 specification.
     * \param cookie Context-specific value.
     * \retval ::FWK_SUCCESS or one of FWK_E_* error codes.
     */
    int (*set_reset_state)(fwk_id_t element_id,
                           enum mod_reset_domain_mode mode,
                           uint32_t reset_state,
                           uintptr_t cookie);
};

/*!
 * \brief Reset domain driver interface.
 *
 * \details The interface this reset domain module relies on to perform
 *      actions on a reset domain.
 */
struct mod_reset_domain_drv_api {
    /*!
     * \brief Change reset state of the device \p dev_id
     *
     * \param dev_id Reset domain driver identifier.
     * \param mode Reset domain mode.
     * \param reset_state Reset domain state as defined in SCMIv2 specification.
     *
     * \param cookie Context-specific value.
     * \retval ::FWK_SUCCESS or one of FWK_E_* error codes.
     */
    int (*set_reset_state)(fwk_id_t dev_id,
                           enum mod_reset_domain_mode mode,
                           uint32_t reset_state,
                           uintptr_t cookie);
};

/*!
 * \brief Reset domain notification indexes.
 */
enum mod_reset_domain_notification_idx {
    /*!
     * \brief Auto reset state change notification index.
     */
    MOD_RESET_DOMAIN_NOTIFICATION_AUTORESET,

    /*!
     * \brief Number of notifications available.
     */
    MOD_RESET_DOMAIN_NOTIFICATION_IDX_COUNT
};


/*!
 * \brief Reset domain auto reset notification event parameters.
 */
struct mod_reset_domain_notification_event_params {
    /*!
     * \brief Domain identifier associated with the notification.
     */
    uint32_t domain_id;

    /*!
     * \brief Reset state as defined in SCMIv2 specification.
     */
    uint32_t reset_state;

    /*!
     * \brief Context-specific value(e.g. agent_id) which is returned after
     *     processing a set reset state request. The context-specific value
     *     is the same value which is passed in a set_reset_state call.
     */
    uintptr_t cookie;
};

/*!
 * \brief Reset domain auto reset event parameters.
 */
struct mod_reset_domain_autoreset_event_params {
    /*!
     * \brief Reset device identifier.
     */
    fwk_id_t dev_id;

    /*!
     * \brief Reset state as defined in SCMIv2 specification.
     */
    uint32_t reset_state;

    /*!
     * \brief Context-specific value(e.g. agent_id) which is returned after
     *     processing a set reset state request. The context-specific value
     *     is the same value which is passed in a set_reset_state call.
     */
    uintptr_t cookie;
};

/*!
 * \brief Reset domain event indexes.
 */
enum mod_reset_domain_event_idx {
    /*!
     * \brief Auto reset state change event index.
     */
    MOD_RESET_DOMAIN_EVENT_AUTORESET,

    /*!
     * \brief Total number of events available.
     */
    MOD_RESET_DOMAIN_EVENT_IDX_COUNT
};

/*!
 * Identifier auto reset event.
 *
 * \note The driver will send this event to this module
 *     after completing the auto reset.
 */
static const fwk_id_t mod_reset_domain_autoreset_event_id =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                      MOD_RESET_DOMAIN_EVENT_AUTORESET);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_RESET_DOMAIN_H */
