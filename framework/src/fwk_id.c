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
#include <fwk_id.h>

#include <stdint.h>
#include <stdio.h>

struct __fwk_id_fmt __fwk_id_str(fwk_id_t id)
{
    struct __fwk_id_fmt fmt;

    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

    switch (id.common.type) {
    case FWK_ID_TYPE_MODULE:
        snprintf(fmt.str, sizeof(fmt.str), "[MOD %u]", id.common.module_idx);
        break;

    case FWK_ID_TYPE_ELEMENT:
        snprintf(fmt.str, sizeof(fmt.str), "[ELM %u:%u]", id.element.module_idx,
                 id.element.element_idx);
        break;

    case FWK_ID_TYPE_SUB_ELEMENT:
        snprintf(fmt.str, sizeof(fmt.str), "[SELM %u:%u:%u]",
                 id.sub_element.module_idx, id.sub_element.element_idx,
                 id.sub_element.sub_element_idx);
        break;

    case FWK_ID_TYPE_API:
        snprintf(fmt.str, sizeof(fmt.str), "[API %u:%u]", id.api.module_idx,
                 id.api.api_idx);
        break;

    case FWK_ID_TYPE_EVENT:
        snprintf(fmt.str, sizeof(fmt.str), "[EVT %u:%u]", id.event.module_idx,
                 id.event.event_idx);
        break;

    case FWK_ID_TYPE_NOTIFICATION:
        snprintf(fmt.str, sizeof(fmt.str), "[NOT %u:%u]",
                 id.notification.module_idx, id.notification.notification_idx);
        break;

    default:
        snprintf(fmt.str, sizeof(fmt.str), "<invalid>");

        break;
    }

    return fmt;
}

bool fwk_id_is_type(fwk_id_t id, enum fwk_id_type type)
{
    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.type == type;
}

enum fwk_id_type fwk_id_get_type(fwk_id_t id)
{
    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return id.common.type;
}

bool fwk_id_is_equal(fwk_id_t left, fwk_id_t right)
{
    assert(left.common.type != __FWK_ID_TYPE_INVALID);
    assert(left.common.type < __FWK_ID_TYPE_COUNT);

    return left.value == right.value;
}

fwk_id_t fwk_id_build_module_id(fwk_id_t id)
{
    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_MODULE(id.common.module_idx);
}

fwk_id_t fwk_id_build_element_id(fwk_id_t id, unsigned int element_idx)
{
    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_ELEMENT(id.common.module_idx, element_idx);
}

fwk_id_t fwk_id_build_api_id(fwk_id_t id, unsigned int api_idx)
{
    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

    return FWK_ID_API(id.common.module_idx, api_idx);
}

unsigned int fwk_id_get_module_idx(fwk_id_t id)
{
    assert(id.common.type != __FWK_ID_TYPE_INVALID);
    assert(id.common.type < __FWK_ID_TYPE_COUNT);

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
    assert(sub_element_id.common.type == __FWK_ID_TYPE_SUB_ELEMENT);

    return sub_element_id.sub_element.sub_element_idx;
}

unsigned int fwk_id_get_api_idx(fwk_id_t api_id)
{
    assert(api_id.common.type == __FWK_ID_TYPE_API);

    return api_id.api.api_idx;
}

unsigned int fwk_id_get_event_idx(fwk_id_t event_id)
{
    assert(event_id.common.type == __FWK_ID_TYPE_EVENT);

    return event_id.event.event_idx;
}

unsigned int fwk_id_get_notification_idx(fwk_id_t notification_id)
{
    assert(notification_id.common.type == __FWK_ID_TYPE_NOTIFICATION);

    return notification_id.notification.notification_idx;
}

static_assert(sizeof(fwk_id_t) == sizeof(uint32_t),
    "fwk_id_t has invalid size");
