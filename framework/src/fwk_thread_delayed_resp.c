/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_module.h>

#include <fwk_element.h>
#include <fwk_host.h>
#include <fwk_interrupt.h>

#include <stdint.h>

#ifdef BUILD_HOST
static const char err_msg_func[] = "[THR] Error %d in %s\n";
#endif

/*
 * Internal interface functions for use by framework only
 */
struct fwk_slist *__fwk_thread_get_delayed_response_list(fwk_id_t id)
{
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return &__fwk_module_get_ctx(id)->delayed_response_list;

    return &__fwk_module_get_element_ctx(id)->delayed_response_list;
}

struct fwk_event *__fwk_thread_search_delayed_response(
    fwk_id_t id,
    uint32_t cookie)
{
    struct fwk_slist *delayed_response_list;
    struct fwk_slist_node *delayed_response_node;
    struct fwk_event *delayed_response;

    delayed_response_list = __fwk_thread_get_delayed_response_list(id);
    delayed_response_node = fwk_list_head(delayed_response_list);

    while (delayed_response_node != NULL) {
        delayed_response =
            FWK_LIST_GET(delayed_response_node, struct fwk_event, slist_node);
        if (delayed_response->cookie == cookie)
            return delayed_response;

        delayed_response_node =
            fwk_list_next(delayed_response_list, delayed_response_node);
    }

    return NULL;
}

/*
 * Public Interface functions for use by Modules
 */
int fwk_thread_get_delayed_response(
    fwk_id_t id,
    uint32_t cookie,
    struct fwk_event *event)
{
    int status = FWK_E_PARAM;
    struct fwk_event *delayed_response;
    unsigned int interrupt;

    if (fwk_interrupt_get_current(&interrupt) == FWK_SUCCESS) {
        status = FWK_E_ACCESS;
        goto error;
    }

    if (!fwk_module_is_valid_entity_id(id))
        goto error;

    if (event == NULL)
        goto error;

    delayed_response = __fwk_thread_search_delayed_response(id, cookie);
    if (delayed_response == NULL)
        goto error;

    *event = *delayed_response;

    return FWK_SUCCESS;

error:
    FWK_HOST_PRINT(err_msg_func, status, __func__);
    return status;
}
