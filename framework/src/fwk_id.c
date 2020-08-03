/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Identifiers.
 */

#include <internal/fwk_id.h>

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void fwk_id_format(
    char *buffer,
    size_t buffer_size,
    fwk_id_t id,
    bool verbose)
{
    static const char *types[] = {
        [__FWK_ID_TYPE_INVALID] = "INV",
        [__FWK_ID_TYPE_NONE] = "NON",
        [__FWK_ID_TYPE_MODULE] = "MOD",
        [__FWK_ID_TYPE_ELEMENT] = "ELM",
        [__FWK_ID_TYPE_SUB_ELEMENT] = "SUB",
        [__FWK_ID_TYPE_API] = "API",
        [__FWK_ID_TYPE_EVENT] = "EVT",
        [__FWK_ID_TYPE_NOTIFICATION] = "NOT",
    };

    size_t length = 0;

    fwk_id_t module_id = FWK_ID_NONE_INIT;
    fwk_id_t element_id = FWK_ID_NONE_INIT;

    unsigned int indices[] = { 0, 0, 0 };

    const char *module_name = NULL;
    const char *element_name = NULL;

    fwk_assert(buffer_size > 0);

    if (id.common.type >= FWK_ARRAY_SIZE(types))
        id.common.type = __FWK_ID_TYPE_INVALID;

    indices[0] = id.common.module_idx;

    switch (id.common.type) {
    case __FWK_ID_TYPE_SUB_ELEMENT:
        indices[2] = id.sub_element.sub_element_idx;

        FWK_FALLTHROUGH;

    case __FWK_ID_TYPE_ELEMENT:
        indices[1] = id.element.element_idx;

        break;

    case __FWK_ID_TYPE_API:
        indices[1] = id.api.api_idx;

        break;

    case __FWK_ID_TYPE_EVENT:
        indices[1] = id.event.event_idx;

        break;

    case __FWK_ID_TYPE_NOTIFICATION:
        indices[1] = id.notification.notification_idx;

    default:
        break;
    }

    module_id = FWK_ID_MODULE(indices[0]);
    module_name = fwk_module_get_name(module_id);

    switch (id.common.type) {
    case __FWK_ID_TYPE_ELEMENT:
    case __FWK_ID_TYPE_SUB_ELEMENT:
        element_id = FWK_ID_ELEMENT(indices[0], indices[1]);
        element_name = fwk_module_get_name(element_id);

        break;

    default:
        break;
    }

    length += snprintf(
        buffer + length, buffer_size - length, "[%s", types[id.common.type]);

    switch (id.common.type) {
    case __FWK_ID_TYPE_MODULE:
    case __FWK_ID_TYPE_ELEMENT:
    case __FWK_ID_TYPE_SUB_ELEMENT:
    case __FWK_ID_TYPE_API:
    case __FWK_ID_TYPE_EVENT:
    case __FWK_ID_TYPE_NOTIFICATION:
        if (verbose && (module_name != NULL)) {
            length += snprintf(
                buffer + length, buffer_size - length, " \"%s\"", module_name);
        } else {
            length += snprintf(
                buffer + length, buffer_size - length, " %u", indices[0]);
        }

    default:
        break;
    }

    switch (id.common.type) {
    case __FWK_ID_TYPE_ELEMENT:
    case __FWK_ID_TYPE_SUB_ELEMENT:
    case __FWK_ID_TYPE_API:
    case __FWK_ID_TYPE_EVENT:
    case __FWK_ID_TYPE_NOTIFICATION:
        if (verbose && (element_name != NULL)) {
            length += snprintf(
                buffer + length, buffer_size - length, ":\"%s\"", element_name);
        } else {
            length += snprintf(
                buffer + length, buffer_size - length, ":%u", indices[1]);
        }

    default:
        break;
    }

    switch (id.common.type) {
    case __FWK_ID_TYPE_SUB_ELEMENT:
        length +=
            snprintf(buffer + length, buffer_size - length, ":%u", indices[2]);

    default:
        break;
    }

    length += snprintf(buffer + length, buffer_size - length, "]");
}

struct __fwk_id_fmt __fwk_id_str(fwk_id_t id)
{
    struct __fwk_id_fmt fmt;

    fwk_id_format(fmt.str, sizeof(fmt.str), id, false);

    return fmt;
}

struct fwk_id_verbose_fmt fwk_id_verbose_str(fwk_id_t id)
{
    struct fwk_id_verbose_fmt fmt;

    fwk_id_format(fmt.str, sizeof(fmt.str), id, true);

    return fmt;
}

bool fwk_id_is_type(fwk_id_t id, enum fwk_id_type type)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.type == type;
}

enum fwk_id_type fwk_id_get_type(fwk_id_t id)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.type;
}

bool fwk_id_is_equal(fwk_id_t left, fwk_id_t right)
{
    fwk_assert(left.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(left.common.type < __FWK_ID_TYPE_COUNT);

    return left.value == right.value;
}

fwk_id_t fwk_id_build_module_id(fwk_id_t id)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_MODULE(id.common.module_idx);
}

fwk_id_t fwk_id_build_element_id(fwk_id_t id, unsigned int element_idx)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_ELEMENT(id.common.module_idx, element_idx);
}

fwk_id_t fwk_id_build_api_id(fwk_id_t id, unsigned int api_idx)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_API(id.common.module_idx, api_idx);
}

unsigned int fwk_id_get_module_idx(fwk_id_t id)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.module_idx;
}

unsigned int fwk_id_get_element_idx(fwk_id_t element_id)
{
    assert((element_id.common.type == __FWK_ID_TYPE_ELEMENT) ||
           (element_id.common.type == __FWK_ID_TYPE_SUB_ELEMENT));

    return element_id.element.element_idx;
}

unsigned int fwk_id_get_sub_element_idx(fwk_id_t sub_element_id)
{
    fwk_assert(sub_element_id.common.type == __FWK_ID_TYPE_SUB_ELEMENT);

    return sub_element_id.sub_element.sub_element_idx;
}

unsigned int fwk_id_get_api_idx(fwk_id_t api_id)
{
    fwk_assert(api_id.common.type == __FWK_ID_TYPE_API);

    return api_id.api.api_idx;
}

unsigned int fwk_id_get_event_idx(fwk_id_t event_id)
{
    fwk_assert(event_id.common.type == __FWK_ID_TYPE_EVENT);

    return event_id.event.event_idx;
}

unsigned int fwk_id_get_notification_idx(fwk_id_t notification_id)
{
    fwk_assert(notification_id.common.type == __FWK_ID_TYPE_NOTIFICATION);

    return notification_id.notification.notification_idx;
}

static_assert(sizeof(fwk_id_t) == sizeof(uint32_t),
    "fwk_id_t has invalid size");
