/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_INTERNAL_ID_H
#define FWK_INTERNAL_ID_H

#include <stdint.h>

/* Identifier type */
enum __fwk_id_type {
    /*
     * Invalid variant.
     *
     * This type is used to catch some classes of missing-initialization errors,
     * and should not be used to initialize new identifiers.
     */
    __FWK_ID_TYPE_INVALID,

    /*
     * None variant.
     *
     * This type is used when an id needs to be explicitly initialized to not
     * refer to any entity.
     */
    __FWK_ID_TYPE_NONE,

    /* Module */
    __FWK_ID_TYPE_MODULE,

    /* Element */
    __FWK_ID_TYPE_ELEMENT,

    /* Sub-element */
    __FWK_ID_TYPE_SUB_ELEMENT,

    /* API */
    __FWK_ID_TYPE_API,

    /* Event */
    __FWK_ID_TYPE_EVENT,

    /* Notification */
    __FWK_ID_TYPE_NOTIFICATION,

    /* Number of defined types */
    __FWK_ID_TYPE_COUNT,
};

/*
 * Generic identifier.
 *
 * This type should be treated as though it is a variant, but where the type
 * switches on all the fields _after_ module_idx. The `type` and `module_idx`
 * fields use the same mask in both variants, but prefer to access them through
 * the \c common field in agnostic code.
 *
 * This identifier fits within the `uint32_t` type, and so should generally be
 * passed by value.
 */
union __fwk_id {
    uint32_t value; /* Integer value */

    struct {
        uint32_t type : 4; /* Identifier type */
        uint32_t module_idx : 8; /* Module index */
        uint32_t reserved : 20; /* Reserved */
    } common; /* Common fields */

    struct {
        uint32_t type : 4; /* Identifier type */
        uint32_t module_idx : 8; /* Module index */
        uint32_t element_idx : 12; /* Element index */
        uint32_t reserved : 8; /* Reserved */
    } element; /* Element variant */

    struct {
        uint32_t type : 4; /* Identifier type */
        uint32_t module_idx : 8; /* Module index */
        uint32_t element_idx : 12; /* Element index */
        uint32_t sub_element_idx : 8; /* Sub-element index */
    } sub_element; /* Sub-element variant */

    struct {
        uint32_t type : 4; /* Identifier type */
        uint32_t module_idx : 8; /* Module index */
        uint32_t api_idx : 4; /* API index */
        uint32_t reserved : 16; /* Reserved */
    } api; /* API variant */

    struct {
        uint32_t type : 4; /* Identifier type */
        uint32_t module_idx : 8; /* Module index */
        uint32_t event_idx : 6; /* Event index */
        uint32_t reserved : 14; /* Reserved */
    } event; /* Event variant */

    struct {
        uint32_t type : 4; /* Identifier type */
        uint32_t module_idx : 8; /* Module index */
        uint32_t notification_idx : 6; /* Notification index */
        uint32_t reserved : 14; /* Reserved */
    } notification; /* Notification variant */
};

/*
 * Print format helper structure.
 *
 * This structure is necessary to prevent the string buffer from decaying to
 * a pointer when returned from __fwk_id_str(). This ensures the string
 * buffer is kept in scope for the entire expression, as opposed to going out of
 * scope once __fwk_id_str() returns.
 */
struct __fwk_id_fmt {
    char str[20]; /* Identifier string representation */
};

/*
 * Build the string representation of an identifier.
 *
 * \param id Identifier.
 *
 * \return Buffer structure containing the string representation of the
 *      identifier.
 */
struct __fwk_id_fmt __fwk_id_str(union __fwk_id id);

#endif /* FWK_INTERNAL_ID_H */
