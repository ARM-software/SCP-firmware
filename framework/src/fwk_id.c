/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
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

static void fwk_id_format(char *buffer, size_t buffer_size, fwk_id_t id)
{
    static const char *types[__FWK_ID_TYPE_COUNT] = {
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

    enum __fwk_id_type com_id_type = (enum __fwk_id_type)id.common.type;

    unsigned int indices[] = { 0, 0, 0 };

    fwk_assert(buffer_size > 0);

    if (id.common.type >= FWK_ARRAY_SIZE(types) ||
        id.common.type == (uint32_t)__FWK_ID_TYPE_INVALID) {
        (void)snprintf(
            buffer,
            buffer_size,
            "[%s]",
            types[(uint32_t)__FWK_ID_TYPE_INVALID]);
        return;
    }

    indices[0] = id.common.module_idx;

    switch (com_id_type) {
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

        break;

    default:
        break;
    }

    length += snprintf(
        buffer + length,
        buffer_size - length,
        "[%s %u",
        types[id.common.type],
        indices[0]);

    switch (com_id_type) {
    case __FWK_ID_TYPE_ELEMENT:
    case __FWK_ID_TYPE_SUB_ELEMENT:
    case __FWK_ID_TYPE_API:
    case __FWK_ID_TYPE_EVENT:
    case __FWK_ID_TYPE_NOTIFICATION:
        length +=
            snprintf(buffer + length, buffer_size - length, ":%u", indices[1]);

        break;

    default:
        break;
    }

    switch (com_id_type) {
    case __FWK_ID_TYPE_SUB_ELEMENT:
        length +=
            snprintf(buffer + length, buffer_size - length, ":%u", indices[2]);

        break;

    default:
        break;
    }

    (void)snprintf(buffer + length, buffer_size - length, "]");
}

struct __fwk_id_fmt __fwk_id_str(fwk_id_t id)
{
    struct __fwk_id_fmt fmt = { { 0 } };

    fwk_id_format(fmt.str, sizeof(fmt.str), id);

    return fmt;
}

bool fwk_id_is_type(fwk_id_t id, enum fwk_id_type type)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.type == type;
}

bool fwk_id_type_is_valid(fwk_id_t id)
{
    if ((id.common.type != __FWK_ID_TYPE_INVALID) &&
        (id.common.type < __FWK_ID_TYPE_COUNT)) {
        return true;
    }

    return false;
}

enum fwk_id_type fwk_id_get_type(fwk_id_t id)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return (enum fwk_id_type)id.common.type;
}

bool fwk_id_is_equal(fwk_id_t left, fwk_id_t right)
{
    fwk_assert(left.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(left.common.type < __FWK_ID_TYPE_COUNT);

    return left.value == right.value;
}

bool fwk_optional_id_is_defined(fwk_optional_id_t id)
{
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);
    return id.common.type != __FWK_ID_TYPE_INVALID;
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

fwk_id_t fwk_id_build_sub_element_id(fwk_id_t id, unsigned int sub_element_idx)
{
    fwk_assert(id.common.type == __FWK_ID_TYPE_ELEMENT);

    return FWK_ID_SUB_ELEMENT(
        id.common.module_idx, id.element.element_idx, sub_element_idx);
}

fwk_id_t fwk_id_build_api_id(fwk_id_t id, unsigned int api_idx)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_API(id.common.module_idx, api_idx);
}

/*
 * Following functions are enabled only for debug build, release build
 * will use inline equivalents, see fwk_id.h
 */
#if !defined(NDEBUG)
unsigned int fwk_id_get_module_idx(fwk_id_t id)
{
    fwk_assert(id.common.type != __FWK_ID_TYPE_INVALID);
    fwk_assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.module_idx;
}

unsigned int fwk_id_get_element_idx(fwk_id_t element_id)
{
    fwk_assert(
        (element_id.common.type == __FWK_ID_TYPE_ELEMENT) ||
        (element_id.common.type == __FWK_ID_TYPE_SUB_ELEMENT));

    return element_id.element.element_idx;
}

unsigned int fwk_id_get_sub_element_idx(fwk_id_t sub_element_id)
{
    fwk_assert(sub_element_id.common.type == __FWK_ID_TYPE_SUB_ELEMENT);

    return sub_element_id.sub_element.sub_element_idx;
}
#endif

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
