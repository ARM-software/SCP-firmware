/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *    Framework private module definitions.
 */

#ifndef FWK_INTERNAL_MODULE_H
#define FWK_INTERNAL_MODULE_H

#include <internal/fwk_notification.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_slist.h>

#include <stddef.h>

/*
 * Module context.
 */
struct fwk_module_context {
    /* Module identifier */
    fwk_id_t id;

    /* Module state */
    enum fwk_module_state state;

    /* Module description */
    const struct fwk_module *desc;

    /* Module configuration */
    const struct fwk_module_config *config;

    /* Number of elements */
    size_t element_count;

    /* Table of element contexts */
    struct fwk_element_ctx *element_ctx_table;

#ifdef BUILD_HAS_NOTIFICATION
    /*
     * Table of notification subscription lists. One list per type of
     * notification defined by the module.
     */
    struct fwk_dlist *subscription_dlist_table;
    #endif

    /* List of delayed response events */
    struct fwk_slist delayed_response_list;
};

/*
 * Element context.
 */
struct fwk_element_ctx {
    /* Element state */
    enum fwk_module_state state;

    /* Element description */
    const struct fwk_element *desc;

    /* Number of sub-elements */
    size_t sub_element_count;

#ifdef BUILD_HAS_NOTIFICATION
    /*
     * Table of notification subscription lists. One list per type of
     * notification defined by the element's module.
     */
    struct fwk_dlist *subscription_dlist_table;
    #endif

    /* List of delayed response events */
    struct fwk_slist delayed_response_list;
};

/*!
 * \internal
 *
 * \brief Start the module component.
 *
 * \details Starts each module and its elements, running through their
 *      initialization routines.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_module_start(void);

/*!
 * \internal
 *
 * \brief Stop the module component.
 *
 * \details Stop each module and its elements, running through their
 *      stop routines.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_module_stop(void);

/*
 * \brief Get a pointer to the context of a module or element.
 *
 * \param id Module or element identifier.
 *      If the identifier provided does not refer to a valid module or element,
 *      the behaviour of this function is undefined.
 *
 * \return Pointer to the module context.
 */
struct fwk_module_context *fwk_module_get_ctx(fwk_id_t id);

/*
 * \brief Get the state of a module or element.
 *
 * \param id Module, element or sub-element identifier.
 * \param state [out] State of the module or element.
 *
 * \retval ::FWK_SUCCESS The state was returned.
 * \retval ::FWK_E_PARAM One or more parameters were invalid.
 */
int fwk_module_get_state(fwk_id_t id, enum fwk_module_state *state);

/*
 * \brief Get a pointer to the framework context of an element.
 *
 * \param element_id Element identifier.
 *      If the identifier provided does not refer to a valid element, the
 *      behaviour of this function is undefined.
 *
 * \return Pointer to the element context.
 */
struct fwk_element_ctx *fwk_module_get_element_ctx(fwk_id_t element_id);

/*
 * \brief Reset the module framework component.
 *
 * \note Only for testing.
 */
void fwk_module_reset(void);

#endif /* FWK_INTERNAL_MODULE_H */
