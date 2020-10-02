/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain management support.
 */

#ifndef MOD_POWER_DOMAIN_H
#define MOD_POWER_DOMAIN_H

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupPowerDomain Power Domains
 *
 * \details Support for querying and setting the power state of power domains
 *      such as CPU, cluster and GPU power domains.
 *
 * \{
 */

/*!
 * Maximum number of states for a power domain.
 */
#define MOD_PD_STATE_COUNT_MAX 16

/*!
 * \brief Types of power domain.
 */
enum mod_pd_type {
    /*! Processor. */
    MOD_PD_TYPE_CORE,
    /*! Processor cluster. */
    MOD_PD_TYPE_CLUSTER,
    /*! Generic device. */
    MOD_PD_TYPE_DEVICE,
    /*! Debug device. */
    MOD_PD_TYPE_DEVICE_DEBUG,
    /*! System. */
    MOD_PD_TYPE_SYSTEM,
    /*! Number of power domain types. */
    MOD_PD_TYPE_COUNT
};

/*!
 * \brief Bit width of power domain type within attributes.
 */
#define MOD_PD_TYPE_BITS_WIDTH ((sizeof(unsigned int) * CHAR_BIT) - \
                               __builtin_clz(MOD_PD_TYPE_COUNT - 1))

/*!
 * \brief Attributes of a power domain.
 */
struct mod_pd_attributes {
    /*! Type of the power domain. */
    enum mod_pd_type pd_type : MOD_PD_TYPE_BITS_WIDTH;

    /*! \internal */
    unsigned int not_used : ((sizeof(unsigned int) * CHAR_BIT) -
                             MOD_PD_TYPE_BITS_WIDTH);
};

/*!
 * \brief Identifiers for the power levels.
 */
enum mod_pd_level {
    /*! Level 0. */
    MOD_PD_LEVEL_0,
    /*! Level 1. */
    MOD_PD_LEVEL_1,
    /*! Level 2. */
    MOD_PD_LEVEL_2,
    /*! Level 3. */
    MOD_PD_LEVEL_3,
    /*! Number of power domain levels. */
    MOD_PD_LEVEL_COUNT
};

/*!
 * \brief Power domain module configuration.
 */
struct mod_power_domain_config {
    /*!
     *  Identifiers of the modules and elements which are granted access to the
     *  restricted interface of the module.
     */
    fwk_id_t *authorized_id_table;

    /*! Number of identifiers in the "authorized_id_table" table. */
    size_t authorized_id_table_size;
};

/*!
 * \brief Configuration data for a power domain.
 *
 * \details The configuration data of a power domain describes the power
 *      domain and its relationship with other power domains in the system.
 *      Assumptions about the organisation of the power domains, underlying the
 *      way in which power domains are described here, are:
 *      - Each power domain can be represented as a node in a power domain
 *        topology tree.
 *      - Sibling power domains are mutually exclusive.
 *      - Parent power domains are shared by the children.
 *
 *      For more information, refer to the "Arm Power State Coordination
 *      Interface Platform Design Document", available here:
 *      http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_
 *      Coordination_Interface_PDD_v1_1_DEN0022D.pdf
 */
struct mod_power_domain_element_config {
    /*!
     * brief Power domain parent index
     */
    uint32_t parent_idx;

    /*!
     *  Identifier of the module or element providing the driver for the power
     *  domain.
     */
    fwk_id_t driver_id;

    /*! Identifier of the driver API. */
    fwk_id_t api_id;

    /*! Attributes of the power domain. */
    struct mod_pd_attributes attributes;

    /*!
     * Mask of the allowed power states for the power domain, for each possible
     * power state of its parent. Table of allowed state masks. The bit 'i' of
     * the entry 'j' is equal to one if and only if the state 'i' for the power
     * domain is allowed when its parent is in state 'j'. The number of bits of
     * each entry of this table has to be greater or equal than
     * MOD_PD_STATE_COUNT_MAX.
     */
    const uint32_t *allowed_state_mask_table;

    /*!
     * Power state names. The power state names are used only for debug
     * purposes and this table is optional. If not provided then the default
     * power state names (OFF, ON, SLEEP, 3, 4, ...) are used for the power
     * domain states.
     */
    const char **state_name_table;

    /*! Composite state number of levels mask */
    const uint32_t composite_state_levels_mask;

    /*!
     * Composite state mask table. This table provides the mask for each level
     */
    const uint32_t *composite_state_mask_table;

    /*! Size of the composite state mask table */
    size_t composite_state_mask_table_size;

    /*! Size of the table of allowed state masks */
    size_t allowed_state_mask_table_size;

    /*! Size of the table of allowed state masks */
    size_t state_name_table_size;

    /*! Disable power domain transition notifications */
    bool disable_state_transition_notifications;
};

/*!
 * \brief Types of system shutdown
 */
enum mod_pd_system_shutdown {
    /*! System shutdown */
    MOD_PD_SYSTEM_SHUTDOWN,

    /*! System cold reset */
    MOD_PD_SYSTEM_COLD_RESET,

    /*! System warm reset */
    MOD_PD_SYSTEM_WARM_RESET,

    /*! Sub-system reset */
    MOD_PD_SYSTEM_SUB_SYSTEM_RESET,

    /*! Forced system shutdown */
    MOD_PD_SYSTEM_FORCED_SHUTDOWN,

    /*! Number of shutdown types */
    MOD_PD_SYSTEM_COUNT,
};

/*!
 * \brief Identifiers of the power domain states. The other states are defined
 *      by the platform code for more flexibility. The power states defined by
 *      the platform must be ordered from the shallowest to the deepest state.
 */
enum mod_pd_state {
    /*! \c OFF power state */
    MOD_PD_STATE_OFF,

    /*! \c ON power state */
    MOD_PD_STATE_ON,

    /*! \c SLEEP power state */
    MOD_PD_STATE_SLEEP,

    /*! Number of power states */
    MOD_PD_STATE_COUNT
};

/*!
 * \brief Masks for the power domain states.
 */
enum mod_pd_state_mask {
    MOD_PD_STATE_OFF_MASK = 1 << MOD_PD_STATE_OFF,
    MOD_PD_STATE_ON_MASK = 1 << MOD_PD_STATE_ON,
    MOD_PD_STATE_SLEEP_MASK = 1 << MOD_PD_STATE_SLEEP,
};

/*!
 * \brief Number of bits for each level state in a composite power state.
 */
#define MOD_PD_CS_STATE_BITS_PER_LEVEL 4

/*!
 * \brief Mask for a composite power state.
 */
#define MOD_PD_CS_STATE_MASK ((1 << MOD_PD_CS_STATE_BITS_PER_LEVEL) - 1)

/*!
 * \brief Shifts for the states and child policies fields in a composite
 *        power state.
 */
enum {
    MOD_PD_CS_LEVEL_0_STATE_SHIFT =
        MOD_PD_LEVEL_0 * MOD_PD_CS_STATE_BITS_PER_LEVEL,
    MOD_PD_CS_LEVEL_1_STATE_SHIFT =
        MOD_PD_LEVEL_1 * MOD_PD_CS_STATE_BITS_PER_LEVEL,
    MOD_PD_CS_LEVEL_2_STATE_SHIFT =
        MOD_PD_LEVEL_2 * MOD_PD_CS_STATE_BITS_PER_LEVEL,
    MOD_PD_CS_LEVEL_3_STATE_SHIFT =
        MOD_PD_LEVEL_3 * MOD_PD_CS_STATE_BITS_PER_LEVEL,
    MOD_PD_CS_LEVEL_SHIFT =
        MOD_PD_LEVEL_COUNT * MOD_PD_CS_STATE_BITS_PER_LEVEL,
};

/*!
 * \brief Compute a composite power domain state.
 */
#define MOD_PD_COMPOSITE_STATE(HIGHEST_LEVEL, LEVEL_3_STATE, LEVEL_2_STATE, \
                               LEVEL_1_STATE, LEVEL_0_STATE)                \
    (((HIGHEST_LEVEL) << MOD_PD_CS_LEVEL_SHIFT)         | \
     ((LEVEL_3_STATE) << MOD_PD_CS_LEVEL_3_STATE_SHIFT) | \
     ((LEVEL_2_STATE) << MOD_PD_CS_LEVEL_2_STATE_SHIFT) | \
     ((LEVEL_1_STATE) << MOD_PD_CS_LEVEL_1_STATE_SHIFT) | \
     ((LEVEL_0_STATE) << MOD_PD_CS_LEVEL_0_STATE_SHIFT))

/*!
 * \brief Power domain driver interface.
 *
 * \details The interface the power domain module relies on to perform
 *      actions on a power domain. Examples include powering a domain on or off,
 *      resetting the domain and preparing for/reacting to a system shutdown or
 *      reset.
 */
struct mod_pd_driver_api {
    /*!
     * \brief Set the power domain identified by \p dev_id to the \p state power
     *      state.
     *
     * \details The power domain module calls this function when processing a
     *      <tt>set state</tt> request. The function is called once the module
     *      has checked that the power domain's children and parent are in a
     *      proper state for the power domain to be transitioned to the \p state
     *      power state. This function must not return until the transition has
     *      completed.
     *
     * \warning In the case of a <tt>shutdown</tt> request, if provided,
     *      ::mod_pd_driver_api::shutdown will be called instead.
     *
     * \param dev_id Driver identifier of the power domain.
     * \param state Power state the power domain has to be put into.
     *
     * \retval ::FWK_SUCCESS The power state has been successfully set.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \return One of the other specific error codes described by the driver
     *      module.
     */
    int (*set_state)(fwk_id_t dev_id, unsigned int state);

    /*!
     * \brief Get the current power state of the power domain identified by
     *      \p dev_id.
     *
     * \details The power domain module calls this function when processing a
     *      <tt>get state</tt> or <tt>get composite state</tt> request.
     *
     * \param dev_id Driver identifier of the power domain.
     * \param[out] state Power state of the power domain.
     *
     * \retval ::FWK_SUCCESS The power state was successfully returned.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     */
    int (*get_state)(fwk_id_t dev_id, unsigned int *state);

    /*!
     * \brief Reset the power domain identified by \p dev_id.
     *
     * \details The power domain module calls this function when processing a
     *      <tt>reset</tt> request. The module will ensure that any children of
     *      the power domain are powered off before attempting a reset.
     *
     * \param dev_id Driver identifier of the power domain.
     *
     * \retval ::FWK_SUCCESS The power domain was successfully reset.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \return One of the other specific error codes described by the module.
     */
    int (*reset)(fwk_id_t dev_id);

    /*!
     * \brief Check whether a state transition request should be denied.
     *
     * \note This function is optional (may be \c NULL) and it is expected that
     *      most drivers will not implement it.
     *
     * \details The power domain module calls this function when it wishes to
     *      know whether it should permit or deny a state transition request.
     *      It is intended to provide a device a denial mechanism that can make
     *      judgements based on information unavailable to the power domain
     *      module, such as dependencies between power domains beyond the power
     *      domain tree model.
     *
     * \param dev_id Driver identifier of the power domain.
     * \param state Power state the power domain is intended to be put into.
     *
     * \retval true The transition should be denied.
     * \retval false The power domain can transition to the power state \p
     *      state.
     */
    bool (*deny)(fwk_id_t dev_id, unsigned int state);

    /*!
     * \brief Prepare the last standing core for a system suspend.
     *
     * \details The function prepares the last standing core for entering the
     *      ::MOD_PD_STATE_OFF state (powered off, no wake-up interrupt) when
     *      it will execute WFI. The function should also ensure that when the
     *      core is powered off a state transition report is sent by means of
     *      the ::mod_pd_driver_input_api::report_power_state_transition
     *      driver input interface function indicating that the core power
     *      domain state should be updated. This function is mandatory for core
     *      power domains, but is otherwise unused.
     *
     * \param dev_id Driver identifier of the power domain.
     *
     * \retval ::FWK_SUCCESS The core has been successfully prepared.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     */
    int (*prepare_core_for_system_suspend)(fwk_id_t dev_id);

    /*!
     * \brief Shutdown a power domain as part of a system shutdown.
     *
     * \note This function is optional (may be \c NULL).
     *
     * \details The power domain module calls this function when processing a
     *      <tt>shutdown</tt> request.
     *
     * \param dev_id Driver identifier of the power domain.
     * \param system_shutdown Type of system shutdown.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_PENDING The operation was acknowledged. Please note that at
     *      present there is no dedicated driver input api for this case.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     */
    int (*shutdown)(fwk_id_t dev_id,
                    enum mod_pd_system_shutdown system_shutdown);
};

/*!
 * \brief Power domain module public interface.
 *
 * \details The interface the power domain module exposes to the other modules
 *      and their elements without any restriction.
 */

struct mod_pd_public_api {
    /*!
     * \brief Get the type of a power domain.
     *
     * \param pd_id Identifier of the power domain that the type is being
     *      queried for.
     * \param[out] type Type of the power domain.
     *
     * \retval ::FWK_SUCCESS The type of the power domain was returned.
     * \retval ::FWK_E_INIT The module has not been initialized.
     * \retval ::FWK_E_STATE The power domain module is in an invalid state.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `pd_id` parameter was not a valid system entity identifier.
     *      - The `type` parameter was a null pointer value.
     */
    int (*get_domain_type)(fwk_id_t pd_id, enum mod_pd_type *type);

    /*!
     * \brief Get the identifier of the parent of a power domain.
     *
     * \note The function returns ::FWK_ID_NONE in the case of the root power
     *      domain which does not have any parent.
     *
     * \param pd_id Identifier of the power domain.
     * \param[out] parent_pd_id Identifier of the parent power domain.
     *
     * \retval ::FWK_SUCCESS The identifier of the parent power domain was
     *      returned.
     * \retval ::FWK_E_INIT The module has not been initialized.
     * \retval ::FWK_E_STATE The power domain module is in an invalid state.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `pd_id` parameter was not a valid system entity identifier.
     *      - The `parent_pd_id` parameter was a null pointer value.
     */
    int (*get_domain_parent_id)(fwk_id_t pd_id, fwk_id_t *parent_pd_id);
};

/*!
 * \brief Power domain module restricted interface.
 *
 * \details The interface the power domain module exposes to a restricted set
 *      of modules and/or elements. The set of modules and/or elements that are
 *      allowed to access this interface is defined by the module configuration
 *      data.
 */
struct mod_pd_restricted_api {
    /*!
     * \brief Get the type of a power domain.
     *
     * \param pd_id Identifier of the power domain that the type is being
     *     queried for.
     * \param[out] type Type of the power domain.
     *
     * \retval ::FWK_SUCCESS The type of the power domain was returned.
     * \retval ::FWK_E_INIT The module has not been initialized.
     * \retval ::FWK_E_STATE The power domain module is in an invalid state.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `pd_id` parameter was not a valid system entity identifier.
     *      - The `type` parameter was a null pointer value.
     */
    int (*get_domain_type)(fwk_id_t pd_id, enum mod_pd_type *type);

    /*!
     * \brief Get the identifier of the parent of a power domain.
     *
     * \note The function returns ::FWK_ID_NONE in the case of the root power
     *      domain which does not have any parent.
     *
     * \param pd_id Identifier of the power domain.
     * \param[out] parent_pd_id The identifier of the parent power domain.
     *
     * \retval ::FWK_SUCCESS The identifier of the parent power domain was
     *      returned.
     * \retval ::FWK_E_INIT The module has not been initialized.
     * \retval ::FWK_E_STATE The power domain module is in an invalid state.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `pd_id` parameter was not a valid system entity identifier.
     *      - The `parent_pd_id` parameter was a null pointer value.
     */
    int (*get_domain_parent_id)(fwk_id_t pd_id, fwk_id_t *parent_pd_id);

    /*!
     * \brief Set the state of a power domain and possibly of one or several of
     *        its ancestors.
     *
     * \note The function sets the state of the power domain identified by
     *      'pd_id' and possibly of one or several of its ancestors. When the
     *      function returns the state transition is completed.
     *
     * \param pd_id Identifier of the power domain whose state has to be set.
     *
     * \param state State the power domain has to be put into and
     *      possibly the state(s) its ancestor(s) has(have) to be put into. The
     *      module will ensure that, for each power state transition, the parent
     *      and the children of the power domain involved are in a state where
     *      the transition can be completed.
     *
     * \retval ::FWK_SUCCESS The power state transition was completed.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_HANDLER The function is not called from a thread.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_state)(fwk_id_t pd_id, uint32_t state);

    /*!
     * \brief Request an asynchronous power state transition.
     *
     * \warning Successful completion of this function does not indicate
     *      completion of a transition, but instead that a request has been
     *      submitted.
     *
     * \param pd_id Identifier of the power domain whose state has to be set.
     *
     * \param resp_requested True if the caller wants to be notified with an
     *      event response at the end of the request processing.
     *
     * \param composite State the power domain has to be put into and
     *      possibly the state(s) its ancestor(s) has(have) to be put into. The
     *      module will ensure that, for each power state transition, the parent
     *      and the children of the power domain involved are in a state where
     *      the transition can be completed.
     *
     * \retval ::FWK_SUCCESS The power state transition was submitted.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_state_async)(fwk_id_t pd_id, bool resp_requested, uint32_t state);

    /*!
     * \brief Get the state of a given power domain.
     *
     * \param pd_id Identifier of the power domain whose state has to be
     *      retrieved.
     * \param[out] state The power domain state.
     *
     * \retval ::FWK_SUCCESS The power state was returned.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_HANDLER The function is not called from a thread.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `pd_id` parameter was not a valid system entity identifier.
     *      - The `state` parameter was a null pointer value.
     */
    int (*get_state)(fwk_id_t pd_id, unsigned int *state);

    /*!
     * \brief Reset of a power domain.
     *
     * \note The function resets the power domain identified by 'pd_id'. When
     *      the function returns the power domain reset is completed.
     *
     * \param pd_id Identifier of the power domain to reset.
     *
     * \retval ::FWK_SUCCESS Power state retrieving request transmitted.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_HANDLER The function is not called from a thread.
     * \retval ::FWK_E_NOMEM Failed to allocate a request descriptor.
     * \retval ::FWK_E_PARAM The power domain identifier is unknown.
     */
    int (*reset)(fwk_id_t pd_id);

    /*!
     * \brief Suspend the system.
     *
     * \note The function initiates the suspension of the system. On call, all
     *      but one core power domain must be in the MOD_PD_STATE_OFF state.
     *      When the function returns the power down of the last standing core
     *      is programmed to occur as soon as the core executes WFI.
     *
     * \param state State the system has to be suspended to. The definition
     *      of those states is platform specific.
     *
     * \retval ::FWK_SUCCESS The system suspension has been initiated
     *      successfully.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_HANDLER The function is not called from a thread.
     * \retval ::FWK_E_NOMEM Failed to allocate a request descriptor.
     * \retval ::FWK_E_PARAM Invalid state.
     * \retval ::FWK_E_STATE The system is not in the proper state to be
     *      suspended.
     */
    int (*system_suspend)(unsigned int state);

    /*!
     * \brief Shutdown the system.
     *
     * \note The function shutdowns the system whatever its current state. If
     *      the shutdown is successful, the function does not return.
     *
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_PENDING Request aknowledged. A response event will not be
     *      sent to the caller.
     * \retval ::FWK_E_HANDLER The function is not called from a thread.
     * \retval ::FWK_E_NOMEM Failed to allocate a request descriptor.
     */
    int (*system_shutdown)(enum mod_pd_system_shutdown system_shutdown);
};

/*!
 * \brief Power domain module driver input API.
 *
 * \details The interface the power domain module exposes to its module drivers
 *      to be able to ask for power state transitions or report power state
 *      transitions following the occurrence of interrupts.
 */
struct mod_pd_driver_input_api {
    /*!
     * \brief Request an asynchronous composite power state transition.
     *
     * \warning Successful completion of this function does not indicate
     *      completion of a transition, but instead that a request has been
     *      submitted.
     *
     * \param pd_id Identifier of the power domain whose state has to be set.
     *
     * \param resp_requested True if the caller wants to be notified with an
     *      event response at the end of the request processing.
     *
     * \param composite_state State the power domain has to be put into and
     *      possibly the state(s) its ancestor(s) has(have) to be put into. The
     *      module will ensure that, for each power state transition, the parent
     *      and the children of the power domain involved are in a state where
     *      the transition can be completed.
     *
     * \retval ::FWK_SUCCESS The composite power state transition was submitted.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     */
    int (*set_state_async)(
        fwk_id_t pd_id,
        bool resp_requested,
        uint32_t composite_state);

    /*!
     * \brief Request for a power domain to be reset.
     *
     * \note The function queues a reset request. When the function returns the
     *      power domain has not been reset, the reset has just been requested
     *      to the power domain module.
     *
     * \param pd_id Identifier of the power domain to reset.
     *
     * \param resp_requested True if the caller wants to be notified with an
     *      event response at the end of the request processing.
     *
     * \retval ::FWK_SUCCESS Reset request transmitted.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_HANDLER The function is not called from a thread.
     * \retval ::FWK_E_NOMEM Failed to allocate a request descriptor.
     * \retval ::FWK_E_PARAM The power domain identifier is unknown.
     */
    int (*reset_async)(fwk_id_t pd_id, bool resp_requested);

    /*!
     * \brief Report a power domain state transition.
     *
     * \warning Failure to call this function on a power state transition may
     *      leave the power domain module with an outdated power state for the
     *      power domain.
     *
     * \param pd_id Identifier of the power domain, a power state transition
     *      report is is sent to.
     *
     * \retval ::FWK_SUCCESS Report transmitted.
     * \retval ::FWK_E_ACCESS Invalid access, the framework has rejected the
     *      call to the API.
     * \retval ::FWK_E_NOMEM Failed to allocate a report event.
     * \retval ::FWK_E_PARAM The power domain identifier is unknown.
     */
    int (*report_power_state_transition)(fwk_id_t pd_id, unsigned int state);

    /*!
     * \brief Get the power domain identifier of the last core online before or
     *      during system suspend.
     *
     * \details When an agent calls for system suspend, the power domain module
     *      ensures that all cores are turned off except for one, which is the
     *      calling entity. Because any core can call system suspend, it may be
     *      necessary for a driver to know which core called system suspend.
     *      With this information the driver can, for example, safely resume the
     *      system from the same core which was turned off lastly.
     *
     * \note The power domain module does not perform any checks to ensure that
     *      this function is called during a system suspend sequence.
     *
     * \param[out] last_core_pd_id Identifier of the last core.
     *
     * \retval ::FWK_E_PARAM The pointer to the identifier is not valid.
     * \retval ::FWK_SUCCESS The request was successful.
     * \return One of the standard framework error codes.
     */
    int (*get_last_core_pd_id)(fwk_id_t *last_core_pd_id);
};

/*!
 * \brief Parameters of a power state pre-transition notification.
 */
struct mod_pd_power_state_pre_transition_notification_params {
    /*! Current power state */
    unsigned int current_state;

    /*! Target power state */
    unsigned int target_state;
};

/*!
 * \brief Parameters of the response to a power state pre-transition
 *      notification.
 */
struct mod_pd_power_state_pre_transition_notification_resp_params {
    /*!
     *  Status of the transition for the entity being notified.
     */
    int status;
};

/*!
 * \brief Parameters of a power domain transition notification.
 */
struct mod_pd_power_state_transition_notification_params {
    /*! Power state the power domain has transitioned to */
    unsigned int state;
};

/*!
 * \brief Parameters of a pre-shutdown transition notification.
 */
struct mod_pd_pre_shutdown_notif_params {
    /*! System shutdown type the system is transitioning to */
    enum mod_pd_system_shutdown system_shutdown;
};

/*!
 * \brief Parameters of the response to a pre-shutdown transition notification.
 */
struct mod_pd_pre_shutdown_notif_resp_params {
    /*! Status of the operation from the entity being notified */
    int status;
};

/*!
 * \defgroup GroupPowerDomainIds Identifiers
 * \{
 */

/*!
 * \brief API indices
 */
enum mod_pd_api_idx {
    MOD_PD_API_IDX_PUBLIC,
    MOD_PD_API_IDX_RESTRICTED,
    MOD_PD_API_IDX_DRIVER_INPUT,
    MOD_PD_API_IDX_COUNT,
};

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
/*! Public API identifier */
static const fwk_id_t mod_pd_api_id_public =
    FWK_ID_API_INIT(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_PUBLIC);

/*! Restricted API identifier */
static const fwk_id_t mod_pd_api_id_restricted =
    FWK_ID_API_INIT(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED);

/*! Driver input API identifier */
static const fwk_id_t mod_pd_api_id_driver_input =
    FWK_ID_API_INIT(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_DRIVER_INPUT);
#endif

/*!
 * \brief Notification indices.
 */
enum mod_pd_notification_idx {
    /*! Power state transition */
    MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION,

    /*! Power state pre-transition */
    MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION,

    /*! Broadcast notification before shutdown starts */
    MOD_PD_NOTIFICATION_IDX_PRE_SHUTDOWN,

    /*! Number of notifications defined by the power domain module */
    MOD_PD_NOTIFICATION_COUNT,
};

/*!
 * \brief Notification identifiers.
 */
#ifdef BUILD_HAS_MOD_POWER_DOMAIN
/*! Identifier of the power state transition notification */
static const fwk_id_t mod_pd_notification_id_power_state_transition =
    FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                             MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION);

/*! Identifier of the power state pre-transition notification */
static const fwk_id_t mod_pd_notification_id_power_state_pre_transition =
    FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
        MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION);

/*!
 * Identifier of the pre-shutdown notification.
 *
 * \note This notification will be broadcast with module identifier only.
 */
static const fwk_id_t mod_pd_notification_id_pre_shutdown =
    FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
        MOD_PD_NOTIFICATION_IDX_PRE_SHUTDOWN);

#endif

/*!
 * \brief Power Domain public events indices
 */
enum mod_pd_public_event_idx {
    /*! Set state request event */
    MOD_PD_PUBLIC_EVENT_IDX_SET_STATE,

    /*! Get state request event */
    MOD_PD_PUBLIC_EVENT_IDX_GET_STATE,

    /*! Number of public Power Domain events */
    MOD_PD_PUBLIC_EVENT_IDX_COUNT,
};

/*!
 * \brief Parameters of the set state response event
 */
struct pd_set_state_response {
    /*! Status of the set state request event processing */
    int status;

    /*!
     * \brief Copy of the "composite_state" request parameter
     *
     * \details The composite state that defines the power state that the power
     *      domain, target of the request, had to be put into and possibly the
     *      power states the ancestors of the power domain had to be put into.
     */
    uint32_t composite_state;
};

/*! Parameters of the get state response event */
struct pd_get_state_response {
    /*! Status of the get state request event processing */
    int status;

    /*! Copy of the "composite" request parameter */
    bool composite;

    /*!
     * \brief Copy of the "state" request parameter
     *
     * \details The power state of the power domain target of the request or
     *      the composite state of the power domain and its ancestors depending
     *      on the value of the "composite" request parameter.
     */
    uint32_t state;
};

/*!
 * \brief Public Events identifiers.
 */
#ifdef BUILD_HAS_MOD_POWER_DOMAIN
/*! Identifier of the public event set_state identifier */
static const fwk_id_t mod_pd_public_event_id_set_state =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                      MOD_PD_PUBLIC_EVENT_IDX_SET_STATE);

/*! Identifier of the public event get_state identifier */
static const fwk_id_t mod_pd_public_event_id_get_state =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                      MOD_PD_PUBLIC_EVENT_IDX_GET_STATE);
#endif

/*!
 * \}
 */

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_POWER_DOMAIN_H */
