/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_module.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const char err_msg_func[] = "[FWK] Error %d in %s";

/*
 * Static functions
 */
static int check_api_call(fwk_id_t id, void *data)
{
    unsigned int interrupt;

    if (fwk_interrupt_get_current(&interrupt) == FWK_SUCCESS)
        return FWK_E_ACCESS;

    if ((!fwk_module_is_valid_entity_id(id)) || (data == NULL))
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

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
    int status;
    struct fwk_event *delayed_response;

    status = check_api_call(id, event);
    if (status != FWK_SUCCESS)
        goto error;

    delayed_response = __fwk_thread_search_delayed_response(id, cookie);
    if (delayed_response == NULL) {
        status = FWK_E_PARAM;
        goto error;
    }

    *event = *delayed_response;

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int fwk_thread_is_delayed_response_list_empty(
    fwk_id_t id,
    bool *is_empty)
{
    int status;
    struct fwk_slist *delayed_response_list;
    struct fwk_slist_node *delayed_response_node;

    status = check_api_call(id, is_empty);
    if (status != FWK_SUCCESS)
        goto error;

    delayed_response_list = __fwk_thread_get_delayed_response_list(id);
    delayed_response_node = fwk_list_head(delayed_response_list);

    *is_empty = (delayed_response_node == NULL);

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int fwk_thread_get_first_delayed_response(
    fwk_id_t id,
    struct fwk_event *event)
{
    int status;
    struct fwk_slist *delayed_response_list;
    struct fwk_slist_node *delayed_response_node;

    status = check_api_call(id, event);
    if (status != FWK_SUCCESS)
        goto error;

    delayed_response_list = __fwk_thread_get_delayed_response_list(id);
    delayed_response_node = fwk_list_head(delayed_response_list);

    if (delayed_response_node != NULL) {
        *event =
           *(FWK_LIST_GET(delayed_response_node, struct fwk_event, slist_node));
    } else
        return FWK_E_STATE;

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}
