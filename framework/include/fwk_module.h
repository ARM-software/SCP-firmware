/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Module definitions.
 */

#ifndef FWK_MODULE_H
#define FWK_MODULE_H

#include <fwk_element.h>
#include <fwk_event.h>
#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupModule Modules
 * @{
 */

/*!
 * \brief Module types.
 */
enum fwk_module_type {
    /*! Hardware Abstraction Layer */
    FWK_MODULE_TYPE_HAL,
    /*! Device driver */
    FWK_MODULE_TYPE_DRIVER,
    /*! Protocol */
    FWK_MODULE_TYPE_PROTOCOL,
    /*! Service provider */
    FWK_MODULE_TYPE_SERVICE,
    /*! The number of defined module types */
    FWK_MODULE_TYPE_COUNT
};

/*!
 * \brief Module or element state flags.
 */
enum fwk_module_state {
    /*! The module or element has not yet been initialized */
    FWK_MODULE_STATE_UNINITIALIZED = 0,

    /*! The module or element has been initialized successfully */
    FWK_MODULE_STATE_INITIALIZED,

    /*! The module or element has bound successfully */
    FWK_MODULE_STATE_BOUND,

    /*! The module or element has started successfully */
    FWK_MODULE_STATE_STARTED,

    /*! The module or element is suspended */
    FWK_MODULE_STATE_SUSPENDED,

    /*! The number of defined module or element states */
    FWK_MODULE_STATE_COUNT
};

/*!
 * \brief Module descriptor.
 */
struct fwk_module {
    /*! Module name */
    const char *name;

    /*! Module type */
    enum fwk_module_type type;

    /*! Number of APIs defined by the module */
    unsigned int api_count;

    /*! Number of events defined by the module */
    unsigned int event_count;

    #ifdef BUILD_HAS_NOTIFICATION
    /*! Number of notifications defined by the module */
    unsigned int notification_count;
    #endif

    /*!
     * \brief Pointer to the module initialization function.
     *
     * \details This function is invoked during the initialization stage, which
     *      is the first pre-runtime stage. It is called before any module
     *      element initialization is performed. This function must not make any
     *      assumptions about the initialization state of other modules or
     *      their elements.
     *
     * \note This function is \b mandatory and must be implemented by all
     *      modules.
     *
     * \param module_id Identifier of the module being initialized.
     * \param element_count Number of module elements.
     * \param data Module-specific configuration data.
     *
     * \retval FWK_SUCCESS The module was initialized successfully.
     * \retval FWK_E_NOMEM A memory allocation failed.
     * \return One of the other module-defined error codes.
     */
    int (*init)(fwk_id_t module_id, unsigned int element_count,
                const void *data);

    /*!
     * \brief Pointer to the module element initialization function.
     *
     * \details This function is invoked once for each module element during the
     *      initialization stage. Module element initialization occurs after the
     *      call to the module initialization function and before the call to
     *      the module post-initialization function.
     *
     *      Elements are initialized in the order they are declared in the
     *      module configuration data. The initialization function must not
     *      make any assumptions about the initialization state of other modules
     *      and their elements.
     *
     * \note This function is \b mandatory for modules with elements.
     *
     * \param element_id Identifier of the module element being initialized.
     * \param sub_element_count Number of sub-elements.
     * \param data Element-specific configuration data.
     *
     * \retval FWK_SUCCESS The element was initialized successfully.
     * \retval FWK_E_NOMEM A memory allocation failed.
     * \return One of the other module-defined error codes.
     */
    int (*element_init)(fwk_id_t element_id, unsigned int sub_element_count,
         const void *data);

    /*!
     * \brief Pointer to the module post-initialization function.
     *
     * \details This function is invoked to finalize the module initialization
     *      once the module and its elements have been successfully initialized.
     *
     *      The framework does not mandate a particular purpose for this
     *      function. It may be used to deal with dependencies between elements,
     *      for example.
     *
     * \note This function is \b optional.
     *
     * \param module_id Identifier of the module element.
     *
     * \retval FWK_SUCCESS The module post-initialization was successful.
     * \retval FWK_E_NOMEM A memory allocation failed.
     * \return One of the other module-defined error codes.
     */
    int (*post_init)(fwk_id_t module_id);

    /*!
     * \brief Pointer to the bind function.
     *
     * \details This function provides a place for the module and its elements
     *      to bind to other modules and/or elements. Bindings expose APIs
     *      to other modules and elements, and form the core building block of
     *      cross-module interaction.
     *
     *      This function is called by the framework during the bind stage of
     *      the pre-runtime phase. It is called once over the module and then
     *      over all its elements in the initial binding, and a second time to
     *      allow entities with more complex binding strategies to finalize
     *      their bindings.
     *
     *      The first round of calls may be used by a module or element to
     *      discover the entities it needs to bind to, and the second round to
     *      bind to them.
     *
     * \note This function is \b optional.
     *
     * \param id Identifier of the module or element to bind.
     * \param round Current call round, \c 0 for the first round \c 1 for
     *      the second round.
     *
     * \retval FWK_SUCCESS The binding was successful.
     * \retval FWK_E_ACCESS At least one binding request was rejected.
     * \retval FWK_E_NOMEM A memory allocation failed.
     * \return One of the other module-defined error codes.
     */
    int (*bind)(fwk_id_t id, unsigned int round);

    /*!
     * \brief Pointer to the start function.
     *
     * \details This function is called by the framework for the module and then
     *      for all of its elements during the start stage. Elements are
     *      started in the order they are declared in the module configuration
     *      data.
     *
     *      The framework does not mandate a particular purpose for this
     *      function. It may be used to perform any final processing of the
     *      module and its elements before entering the runtime phase. A
     *      possible example of this would be a driver module that must enable
     *      an interrupt after it has completed initialization.
     *
     * \note This function is \b optional.
     *
     * \param id Identifier of the module or element to start.
     *
     * \retval FWK_SUCCESS The module or element was successfully started.
     * \return One of the other module-defined error codes.
     */
    int (*start)(fwk_id_t id);

    /*!
     * \brief Pointer to the bind request processing function.
     *
     * \details This function is called by the framework when it receives a
     *      request from another entity to bind to the module, or an element of
     *      the module. It can be called only during the initialization phase
     *      once 'target_id' has been initialized and during the binding phase.
     *
     *      It allows access control, for instance, to deny certain entities
     *      access. Furthermore, in the case where the module includes different
     *      implementations of an API, it allows the module to select the
     *      implementation to provide based on the requesting entity and/or
     *      target entity.
     *
     * \note This function is \b optional.
     *
     * \param source_id Identifier of the module or element making the
     *      bind request.
     * \param target_id Identifier of the module or element to bind to.
     * \param api_id Identifier of the API to return.
     * \param [out] api Pointer to the API implementation to be used by the
     *      requester.
     *
     * \retval FWK_SUCCESS The binding request was accepted by the module or
     *      element.
     * \retval FWK_E_ACCESS The binding request was rejected by the module or
     *      element.
     * \return One of the other module-defined error codes.
     */
    int (*process_bind_request)(fwk_id_t source_id, fwk_id_t target_id,
                                fwk_id_t api_id, const void **api);

    /*!
     * \brief Process an event.
     *
     * \details This function is called by the framework for events targeting
     *      the module or one of its elements.
     *
     * \note This function is \b optional. If a response event is expected and
     *      the \ref fwk_event.is_delayed_response flag is not set by the
     *      processing function then the response event is immediately and
     *      automatically sent to the event's source by the framework. If
     *      however the \ref fwk_event.is_delayed_response flag is set by the
     *      processing function then the framework does not send the response
     *      event and it is the responsability of the event's target to send it
     *      at some point.
     *
     * \param event Pointer to the event to be processed.
     * \param [out] resp_event The response event to the provided event if
     *      any.
     *
     * \retval FWK_SUCCESS The event was processed successfully.
     * \return One of the other module-defined error codes.
     */
    int (*process_event)(const struct fwk_event *event,
                         struct fwk_event *resp_event);

    /*!
     * \brief Process a notification.
     *
     * \details This function is called by the framework when a notification is
     *      received by a module, element, or sub-element.
     *
     * \note This function is \b optional.
     *
     * \param event Pointer to the notification event to be processed.
     * \param [out] resp_event The response event to the provided event, if
     *      any.
     *
     * \retval FWK_SUCCESS The notification was processed successfully.
     * \return One of the other module-defined error codes.
     */
    int (*process_notification)(const struct fwk_event *event,
                                struct fwk_event *resp_event);
};

/*!
 * \brief Module configuration.
 */
struct fwk_module_config {
    /*!
     * \brief Pointer to the function to get the table of element descriptions.
     *
     * \param module_id Identifier of the module.
     *
     * \details The table of module element descriptions ends with an invalid
     *      element description where the pointer to the element name is
     *      equal to NULL.
     *
     * \warning The framework does not copy the element description data and
     *      keep a pointer to the ones returned by this function. Pointers
     *      returned by this function must thus points to data with static
     *      storage or data stored in memory allocated from the memory
     *      management component.
     */
    const struct fwk_element *(*get_element_table)(fwk_id_t module_id);

    /*! Pointer to the module-specific configuration data */
    const void *data;
};

/*!
 * \brief Check if an identifier refers to a valid module.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid module.
 * \retval false The identifier does not refer to a valid module.
 */
bool fwk_module_is_valid_module_id(fwk_id_t id);

/*!
 * \brief Check if an identifier refers to a valid element.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid element.
 * \retval false The identifier does not refer to a valid element.
 */
bool fwk_module_is_valid_element_id(fwk_id_t id);

/*!
 * \brief Check if an identifier refers to a valid sub-element.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid sub-element.
 * \retval false The identifier does not refer to a valid sub-element.
 */
bool fwk_module_is_valid_sub_element_id(fwk_id_t id);

/*!
 * \brief Check if an identifier refers to a valid module, element or
 *      sub-element.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid module, element or
 *      sub-element.
 * \retval false The identifier does not refer to a valid module, element or
 *      sub-element.
 */
bool fwk_module_is_valid_entity_id(fwk_id_t id);

/*!
 * \brief Check if an identifier refers to a valid API.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid API.
 * \retval false The identifier does not refer to a valid API.
 */
bool fwk_module_is_valid_api_id(fwk_id_t id);

/*!
 * \brief Check if an identifier refers to a valid event.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid event.
 * \retval false The identifier does not refer to a valid event.
 */
bool fwk_module_is_valid_event_id(fwk_id_t id);

/*!
 * \brief Check if an identifier refers to a valid notification.
 *
 * \param id Identifier to be checked.
 *
 * \retval true The identifier refers to a valid notification.
 * \retval false The identifier does not refer to a valid notification.
 */
bool fwk_module_is_valid_notification_id(fwk_id_t id);

/*!
 * \brief Get the number of elements within a module.
 *
 * \param module_id Identifier of the module.
 *
 * \retval FWK_E_PARAM The identifier of the module is invalid.
 * \return Number of module elements.
 */
int fwk_module_get_element_count(fwk_id_t module_id);

/*!
 * \brief Get the number of sub-elements within an element.
 *
 * \param element_id Identifier of the element.
 *
 * \retval FWK_E_PARAM The identifier of the element is invalid.
 * \return Number of sub-elements.
 */
int fwk_module_get_sub_element_count(fwk_id_t element_id);

/*!
 * \brief Get the name of a module or element.
 *
 * \param id Identifier of the module or element.
 *
 * \return The pointer to the module or element name, NULL if the identifier is
 *      not valid.
 */
const char *fwk_module_get_name(fwk_id_t id);

/*!
 * \brief Get the configuration data of a module or element.
 *
 * \param id Identifier of the module or element.
 *
 * \return The pointer to the module/element-specific configuration data, NULL
 *      if the identifier is not valid.
 */
const void *fwk_module_get_data(fwk_id_t id);

/*!
 * \brief Bind to an API of a module or an element.
 *
 * \details The framework will accept the bind request in one of the two
 *      following cases:
 *      1) The execution is at the pre-runtime initialization stage and the
 *         entity 'target_id' has already been initialized.
 *      2) The execution is at the pre-runtime binding stage.
 *
 * \param target_id Identifier of the module or element to bind to.
 * \param api_id Identifier of the API to return an implementation of.
 * \param api [out] Pointer to storage for the pointer to the API.
 *
 * \retval FWK_SUCCESS The API was returned.
 * \retval FWK_E_PARAM A least one of the identifiers is invalid.
 * \retval FWK_E_STATE Call outside of the pre-runtime bind stage.
 * \retval FWK_E_ACCESS The access to the API was refused.
 * \retval FWK_E_HANDLER The returned API pointer is invalid (NULL).
 */
int fwk_module_bind(fwk_id_t target_id, fwk_id_t api_id, const void *api);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_MODULE_H */
