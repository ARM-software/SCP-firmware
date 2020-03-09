/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_DEBUG_H
#define MOD_DEBUG_H

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupDebug Debug HAL
 * \{
 */

/*!
 * \brief Debug events public indices.
 */
enum mod_debug_public_event_idx {
    MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_GET,
    MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_SET,
    MOD_DEBUG_PUBLIC_EVENT_IDX_COUNT
};

/*!
 * \brief Debug subsystems end-users.
 */
enum scp_debug_user {
    /*! Represents another module of the SCP firmware */
    SCP_DEBUG_USER_SCP,

    /*! Represents an agent, the request comes from an AP */
    SCP_DEBUG_USER_AP,

    /*! Represents a debug driver, the request comes from the hardware */
    SCP_DEBUG_USER_DAP,

    /*! Number of SCP Debug users */
    SCP_DEBUG_USER_COUNT
};

/*!
 * \brief Debug API indices.
 */
enum mod_debug_api_idx {
    /*! Index of the Debug HAL API */
    MOD_DEBUG_API_IDX_HAL,

    /*! Index of the Driver input API */
    MOD_DEBUG_API_IDX_DRIVER_INPUT,

    /*! Number of APIs for the Debug module*/
    MOD_DEBUG_API_IDX_COUNT,
};

/*!
 * \brief Debug element configuration data.
 */
struct mod_debug_dev_config {
    /*! Identifier of the debug driver */
    fwk_id_t driver_id;

    /*! Identifier of the debug driver api*/
    fwk_id_t driver_api_id;
};

/*!
 * \brief Debug API.
 */
struct mod_debug_api {
    /*!
     * \brief Enable or disable the Debug functionality.
     *
     * \param id Debug device identifier.
     *
     * \param enable Targeted state for Debug, true for enabled, false
     * otherwise.
     *
     * \param user_id The user requesting the Debug functionality.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The operation has been acknowledged.
     * \retval FWK_E_PARAM One or more parameters were incorrect.
     * \retval FWK_E_BUSY Another request is already being processed.
     * \retval FWK_E_ACCESS The specified user doesn't have the permission to
     * perform the requested action.
     * \return One of the standard framework error codes.
     */
    int (*set_enabled)(fwk_id_t id, bool enable, enum scp_debug_user user_id);

    /*!
     * \brief Get the status of the Debug functionality.
     *
     * \param id Debug device identifier.
     *
     * \param[out] enabled State of Debug, true for enabled, false
     * otherwise.
     *
     * \param user_id The user requesting the Debug enabled status.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The operation has been acknowledged.
     * \retval FWK_E_PARAM One or more parameters were incorrect.
     * \retval FWK_E_BUSY Another request is already being processed.
     * \retval FWK_E_ACCESS The specified user doesn't have the permission to
     * perform the requested action.
     * \return One of the standard framework error codes.
     */
    int (*get_enabled)(fwk_id_t id, bool *enabled, enum scp_debug_user user_id);

    /*!
     * \brief Reset the Debug functionality.
     *
     * \param id Debug device identifier.
     *
     * \retval FWK_E_SUPPORT The operation is not supported.
     * \return One of the standard framework error codes.
     */
    int (*reset)(fwk_id_t id);

    /*!
     * \brief Terminate the Debug functionality.
     *
     * \param id Debug device identifier.
     *
     * \retval FWK_E_SUPPORT The operation is not supported.
     * \return One of the standard framework error codes.
     */
    int (*end)(fwk_id_t id);
};

/*!
 * \brief Debug Driver API.
 */
struct mod_debug_driver_api {
    /*!
     * \brief Enable or disable the driver.
     *
     * \param id Debug driver identifier.
     *
     * \param enable Targeted state the driver true for enabled, false
     * otherwise.
     *
     * \param user_id The user requesting the Debug driver functionality.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The operation has been acknowledged.
     * \retval FWK_E_BUSY Another request is already being processed.
     * \return One of the standard framework error codes.
     */
    int (*set_enabled)(fwk_id_t id, bool enable, enum scp_debug_user user_id);

    /*!
     * \brief Get the status of the driver.
     *
     * \param id Debug driver identifier.
     *
     * \param[out] enabled State of Debug, true for enabled, false
     * otherwise.
     *
     * \param user_id The user requesting the Debug driver enabled status.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The operation has been acknowledged.
     * \retval FWK_E_BUSY Another request is already being processed.
     * \return One of the standard framework error codes.
     */
    int (*get_enabled)(fwk_id_t id, bool *enabled, enum scp_debug_user user_id);
};

/*!
 * \brief Parameters of the response event.
 */
struct mod_debug_response_params {
    /*!
     * \brief Status of the operation requested.
     */
    int status;

    /*!
     * \brief Status of the Debug.
     */
    bool enabled;
};

/*!
 * \brief Debug Driver Input API.
 */
struct mod_debug_driver_input_api {
    /*!
     * \brief Signal the module of a Debug Power request.
     *     This call is expected to come from the DAP user.
     *
     * \param id Debug driver identifier.
     *
     * \param enable Perform the request in enable or disable mode.
     *
     * \param user_id The requesting user.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*enable)(fwk_id_t id, bool enable, enum scp_debug_user user_id);

    /*!
     * \brief Signal the module that the request has been completed.
     *
     * \param id Debug driver identifier.
     *
     * \param response The response data structure.
     */
    void (*request_complete)(fwk_id_t id,
                             struct mod_debug_response_params *response);
};

/*!
 * \brief Get enable event identifier.
 *
 * \details Clients which expect to receive a response event from this module
 *      should use this identifier to properly identify the response.
 */
static const fwk_id_t mod_debug_event_id_req_enable_get =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DEBUG,
                      MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_GET);

/*!
 * \brief Set enable event identifier.
 *
 * \details Clients which expect to receive a response event from this module
 *      should use this identifier to properly identify the response.
 */
static const fwk_id_t mod_debug_event_id_req_enable_set =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DEBUG,
                      MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_SET);

/*!
 * @}
 */
#endif /* MOD_DEBUG_H */
