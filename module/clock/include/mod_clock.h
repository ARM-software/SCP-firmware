/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CLOCK_H
#define MOD_CLOCK_H

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupClock Clock HAL
 *
 * \details A Hardware Abstraction Layer for configuring clock devices.
 *
 * @{
 */

/*!
 * \brief Clock states.
 */
enum mod_clock_state {
    /*! The clock is stopped */
    MOD_CLOCK_STATE_STOPPED,

    /*! The clock is running */
    MOD_CLOCK_STATE_RUNNING,

    /*! Number of defined clock states */
    MOD_CLOCK_STATE_COUNT
};

/*!
 * \brief Clock notification indices.
 */
enum mod_clock_notification_idx {
     /*! The running state of a clock changed */
    MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGED,

     /*! The running state of a clock is about to change */
    MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING,

    /*! Number of defined notifications */
    MOD_CLOCK_NOTIFICATION_IDX_COUNT
};

#if BUILD_HAS_MOD_CLOCK
/*!
 * \brief Identifier for the \ref MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGED
 *     notification.
 */
static const fwk_id_t mod_clock_notification_id_state_changed =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_CLOCK,
        MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGED);

/*!
 * \brief Identifier for the \ref
 *     MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING notification
 */
static const fwk_id_t mod_clock_notification_id_state_change_pending =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_CLOCK,
        MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING);
#endif

/*!
 * \brief Event parameters shared by the
 *     \ref MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGED and
 *     \ref MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING notifications.
 */
struct clock_notification_params {
    /*!
     * The state that the clock has transitioned to, or is about
     * to transition to.
     */
    enum mod_clock_state new_state;
};

/*!
 * \brief Response parameters for the
 *     \ref MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING notification.
 */
struct clock_state_change_pending_resp_params {
    /*!
     * The status returned by the notified subscriber on processing the
     * \ref MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING notification.
     */
    int status;
};

/*!
 * \brief APIs that the module makes available to entities requesting binding.
 */
enum mod_clock_api_type {
    /*! Clock HAL */
    MOD_CLOCK_API_TYPE_HAL,

    /*! Clock driver response */
    MOD_CLOCK_API_TYPE_DRIVER_RESPONSE,

    /*! Number of defined APIs */
    MOD_CLOCK_API_COUNT,
};

/*!
 * \brief Clock rate types.
 */
enum mod_clock_rate_type {
    /*! The clock has a discrete set of rates that it can attain */
    MOD_CLOCK_RATE_TYPE_DISCRETE,

    /*! The clock has a continuous range of rates with a constant step */
    MOD_CLOCK_RATE_TYPE_CONTINUOUS,
};

/*!
 * \brief Clock rounding modes.
 */
enum mod_clock_round_mode {
    /*!
     * Do not perform any rounding. Any rate that is not precise and attainable
     * will be rejected.
     */
    MOD_CLOCK_ROUND_MODE_NONE,

    /*! Round to the closest attainable rate, whether higher or lower */
    MOD_CLOCK_ROUND_MODE_NEAREST,

    /*! Round to the closest attainable higher rate */
    MOD_CLOCK_ROUND_MODE_UP,

    /*! Round to the closest attainable lower rate */
    MOD_CLOCK_ROUND_MODE_DOWN,
};

/*!
 * \brief Clock module configuration data.
 */
struct mod_clock_config {
    /*!
     * \brief Identifier of a notification to subscribe clock devices to in
     *     order to receive notifications of power domain transitions that have
     *     already occurred.
     *
     * \note May be \ref FWK_ID_NONE to disable this functionality for all
     *     elements.
     */
    const fwk_id_t pd_transition_notification_id;

    /*!
     * \brief Identifier of a notification to subscribe clock devices to in
     *     order to receive notifications of power domain transitions that are
     *     about to occur.
     *
     * \note May be \ref FWK_ID_NONE to disable this functionality for all
     *     elements.
     */
    const fwk_id_t pd_pre_transition_notification_id;
};

/*!
 * \brief Clock element configuration data.
 */
struct mod_clock_dev_config {
    /*! Reference to the device element within the associated driver module */
    const fwk_id_t driver_id;

    /*! Reference to the API provided by the device driver module */
    const fwk_id_t api_id;

    /*!
     * \brief Reference to the element or module that is the source of the
     *     power domain notification.
     *
     * \details If the clock belongs to the always-on (AON) power domain (i.e.
     *     it is always running), or if there are no actions to be performed
     *     when the clock's power domain changes state, then this identifier
     *     must be FWK_ID_NONE. In this case the clock will not be registered
     *     to receive notifications from the power domain module.
     */
    fwk_id_t pd_source_id;
};

/*!
 * \brief Range of supported rates for a clock.
 */
struct mod_clock_range {
    /*! The type of rates the clock provides (discrete or continuous) */
    enum mod_clock_rate_type rate_type;

    /*! Minimum rate (in Hz) */
    uint64_t min;

    /*! Maximum rate (in Hz) */
    uint64_t max;

    /*!
     * The number of Hertz by which the rate can be incremented at each step
     * throughout the clock's range. Valid only when rate_type is equal to
     * \ref mod_clock_rate_type.MOD_CLOCK_RATE_TYPE_CONTINUOUS, as clocks that
     * use \ref mod_clock_rate_type.MOD_CLOCK_RATE_TYPE_DISCRETE may not have a
     * regular step between their rates.
     */
    uint64_t step;

    /*! The number of unique rates that the clock can attain */
    uint64_t rate_count;
};

/*!
 * \brief Clock properties exposed via the get_info() API function.
 *
 * \details This structure is intended to store clock information that is static
 *     and which does not change during runtime. Dynamic information, such as
 *     the current clock state, are exposed through functions in the clock and
 *     clock driver APIs.
 */
struct mod_clock_info {
    /*! Human-friendly clock name */
    const char *name;

    /*! Range of supported clock rates */
    struct mod_clock_range range;

    /*! Number of discrete rates supported */
    uint64_t rate_count;
};

/*!
 * \brief Clock driver interface.
 */
struct mod_clock_drv_api {
    /*! Name of the driver */
    const char *name;

    /*!
     * \brief Set a new clock rate by providing a frequency in Hertz (Hz).
     *
     * \param clock_id Clock device identifier.
     *
     * \param rate The desired frequency in Hertz.
     *
     * \param round_mode The type of rounding to perform, if required, to
     *      achieve the given rate.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_rate)(fwk_id_t clock_id, uint64_t rate,
                    enum mod_clock_round_mode round_mode);

    /*!
     * \brief Get the current rate of a clock in Hertz (Hz).
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] rate The current clock rate in Hertz.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_rate)(fwk_id_t clock_id, uint64_t *rate);

    /*!
     * \brief Get a clock rate in Hertz from an index into the clock's range.
     *
     * \param clock_id Clock device identifier.
     *
     * \param rate_index The index into the clock's range to get the rate of.
     *
     * \param[out] rate The rate, in Hertz, corresponding to the index.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_rate_from_index)(fwk_id_t clock_id, unsigned int rate_index,
                               uint64_t *rate);

    /*!
     * \brief Set the running state of a clock.
     *
     * \param clock_id Clock device identifier.
     *
     * \param state One of the valid clock states.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_state)(fwk_id_t clock_id, enum mod_clock_state state);

    /*!
     * \brief Get the running state of a clock.
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] state The current clock state.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_state)(fwk_id_t clock_id, enum mod_clock_state *state);

    /*!
     * \brief Get the range of rates that the clock supports.
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] range The clock range structure.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_range)(fwk_id_t clock_id, struct mod_clock_range *range);

    /*!
     * \brief Handle the condition where the state of a clock's power domain is
     *     about to change.
     *
     * \details This function will be called prior to the change in power
     *     state occurring so that the clock driver implementing this API is
     *     able to perform any required preparatory work beforehand.
     *
     * \note This function is optional. If the driver does not control any
     *     clocks that require power state awareness then the pointer may be set
     *     to NULL.
     *
     * \param clock_id Clock device identifier.
     *
     * \param current_state The current power state that the clock's power
     *     domain will transition away from.
     *
     * \param new_state The power state that the clock's power domain will
     *     transition to.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*process_pending_power_transition)(
        fwk_id_t clock_id,
        unsigned int current_state,
        unsigned int new_state);

    /*!
     * \brief Handle the condition where the state of a clock's power domain
     *     has changed.
     *
     * \details This function will be called after the change in power state
     *     has occurred. The driver can take any appropriate actions that are
     *     required to accommodate the new state. The transition can be to a
     *     deeper power state (e.g. ON->OFF) or to a shallower power state
     *     (e.g. OFF->ON).
     *
     * \note This function is optional. If the driver does not control any
     *     clocks that require power state awareness then the pointer may be set
     *     to NULL.
     *
     * \param clock_id Clock device identifier.
     *
     * \param state The power state that the clock's power domain transitioned
     *     to.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*process_power_transition)(fwk_id_t clock_id, unsigned int state);
};

/*!
 * \brief Clock interface.
 */
struct mod_clock_api {
    /*!
     * \brief Set a new clock rate by providing a frequency in Hertz (Hz).
     *
     * \param clock_id Clock device identifier.
     *
     * \param rate The desired frequency in Hertz.
     *
     * \param round_mode The type of rounding to perform, if required, to
     *      achieve the given rate.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The result for this operation
     *      will be provided via a response event.
     * \retval FWK_E_PARAM The clock identifier was invalid.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*set_rate)(fwk_id_t clock_id, uint64_t rate,
                    enum mod_clock_round_mode round_mode);

    /*!
     * \brief Get the current rate of a clock in Hertz (Hz).
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] rate The current clock rate in Hertz.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The requested rate will be
     *      provided via a response event.
     * \retval FWK_E_PARAM The clock identifier was invalid.
     * \retval FWK_E_PARAM The rate pointer was NULL.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*get_rate)(fwk_id_t clock_id, uint64_t *rate);

    /*!
     * \brief Get a clock rate in Hertz from an index into the clock's range.
     *
     * \param clock_id Clock device identifier.
     *
     * \param rate_index The index into the clock's range to get the rate of.
     *
     * \param[out] rate The rate, in Hertz, corresponding to the index.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The clock identifier was invalid.
     * \retval FWK_E_PARAM The rate pointer was NULL.
     * \return One of the standard framework error codes.
     */
    int (*get_rate_from_index)(fwk_id_t clock_id, unsigned int rate_index,
                               uint64_t *rate);

    /*!
     * \brief Set the running state of a clock.
     *
     * \param clock_id Clock device identifier.
     *
     * \param state One of the valid clock states.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The result for this operation
     *      will be provided via a response event.
     * \retval FWK_E_PARAM The clock identifier was invalid.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*set_state)(fwk_id_t clock_id, enum mod_clock_state state);

    /*!
     * \brief Get the running state of a clock.
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] state The current clock state.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The requested state will be
     *      provided via a response event.
     * \retval FWK_E_PARAM The clock identifier was invalid.
     * \retval FWK_E_PARAM The state pointer was NULL.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*get_state)(fwk_id_t clock_id, enum mod_clock_state *state);

    /*!
     * \brief Get information about a clock's fixed properties.
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] info The clock device properties.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The clock identifier was invalid.
     * \retval FWK_E_PARAM The info pointer was NULL.
     * \return One of the standard framework error codes.
     */
    int (*get_info)(fwk_id_t clock_id, struct mod_clock_info *info);
};

/*!
 * \brief Container for the values returned upon request completion.
 */
union mod_clock_resp_values {
    /*! The current clock rate in Hertz */
    uint64_t rate;

    /*! The current clock state */
    enum mod_clock_state state;
};

/*!
 * \brief Driver response parameters.
 */
struct mod_clock_driver_resp_params {
    /*! Status of driver operation */
    int status;

    /*! Values returned */
    union mod_clock_resp_values value;
};

/*!
 * \brief Clock driver response API.
 *
 * \details API used by the driver when an asynchronous request is completed.
 *
 */
struct mod_clock_driver_response_api {
    /*!
     * \brief Signal the completion of a driver request.
     *
     * \param dev_id Specific clock device identifier.
     * \param resp_values Pointer to the values requested.
     */
    void (*request_complete)(fwk_id_t dev_id,
                             struct mod_clock_driver_resp_params *resp_values);
};

/*!
 * \brief Event response parameters.
 */
struct mod_clock_resp_params {
    /*! Status of requested operation */
    int status;

    /*! Values returned */
    union mod_clock_resp_values value;
};

/*!
 * \brief Define the first exposed event handled by this module. Other internal
 *      events take indices after this.
 */
#define MOD_CLOCK_EVENT_IDX_REQUEST     0

/*!
 * \brief Request event identifier.
 *
 * \details This identifier is used by the clients that expect to receive a
 *      response event from this module.
 */
static const fwk_id_t mod_clock_event_id_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_CLOCK, MOD_CLOCK_EVENT_IDX_REQUEST);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CLOCK_H */
