/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Single-thread facilities.
 */

#include <internal/fwk_module.h>
#include <internal/fwk_single_thread.h>
#include <internal/fwk_thread.h>
#include <internal/fwk_thread_delayed_resp.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_noreturn.h>
#include <fwk_slist.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <string.h>

static struct __fwk_thread_ctx ctx;

static const char err_msg_line[] = "[FWK] Error %d in %s @%d";
static const char err_msg_func[] = "[FWK] Error %d in %s";

/*
 * Static functions
 */

/*
 * Duplicate an event.
 *
 * \param event Pointer to the event to duplicate.
 *
 * \pre \p event must not be NULL
 *
 * \return The pointer to the duplicated event, NULL if the allocation to
 *      duplicate the event failed.
 */
static struct fwk_event *duplicate_event(struct fwk_event *event)
{
    struct fwk_event *allocated_event = NULL;

    fwk_assert(event != NULL);

    fwk_interrupt_global_disable();
    allocated_event = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.free_event_queue), struct fwk_event, slist_node);
    fwk_interrupt_global_enable();

    if (allocated_event == NULL) {
        FWK_LOG_CRIT(err_msg_func, FWK_E_NOMEM, __func__);
        fwk_assert(false);
    }

    *allocated_event = *event;
    allocated_event->slist_node = (struct fwk_slist_node){ 0 };

    return allocated_event;
}

static int put_event(struct fwk_event *event)
{
    struct fwk_event *allocated_event;
    unsigned int interrupt;

    if (event->is_delayed_response) {
        allocated_event = __fwk_thread_search_delayed_response(
            event->source_id, event->cookie);
        if (allocated_event == NULL) {
            FWK_LOG_CRIT(err_msg_func, FWK_E_NOMEM, __func__);
            return FWK_E_PARAM;
        }

        fwk_list_remove(
            __fwk_thread_get_delayed_response_list(event->source_id),
            &allocated_event->slist_node);

        memcpy(
            allocated_event->params,
            event->params,
            sizeof(allocated_event->params));
    } else {
        allocated_event = duplicate_event(event);
        if (allocated_event == NULL)
            return FWK_E_NOMEM;
    }

    allocated_event->cookie = event->cookie = ctx.event_cookie_counter++;

    if (fwk_interrupt_get_current(&interrupt) != FWK_SUCCESS)
        fwk_list_push_tail(&ctx.event_queue, &allocated_event->slist_node);
    else
        fwk_list_push_tail(&ctx.isr_event_queue, &allocated_event->slist_node);

    return FWK_SUCCESS;
}

static void process_next_event(void)
{
    int status;
    struct fwk_event *event, *allocated_event, async_response_event = { 0 };
    const struct fwk_module *module;
    int (*process_event)(const struct fwk_event *event,
                         struct fwk_event *resp_event);


    ctx.current_event = event = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.event_queue), struct fwk_event, slist_node);

    FWK_LOG_TRACE(
        "[FWK] Get event (%s: %s -> %s)\n",
        FWK_ID_STR(event->id),
        FWK_ID_STR(event->source_id),
        FWK_ID_STR(event->target_id));

    module = __fwk_module_get_ctx(event->target_id)->desc;
    process_event = event->is_notification ? module->process_notification :
                    module->process_event;

    if (event->response_requested) {
        async_response_event = *event;
        async_response_event.source_id = event->target_id;
        async_response_event.target_id = event->source_id;
        async_response_event.is_delayed_response = false;

        status = process_event(event, &async_response_event);
        if (status != FWK_SUCCESS)
            FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);

        async_response_event.is_response = true;
        async_response_event.response_requested = false;
        if (!async_response_event.is_delayed_response)
            put_event(&async_response_event);
        else {
            allocated_event = duplicate_event(&async_response_event);
            if (allocated_event != NULL) {
                fwk_list_push_tail(
                    __fwk_thread_get_delayed_response_list(
                        async_response_event.source_id),
                    &allocated_event->slist_node);
            }
        }
    } else {
        status = process_event(event, &async_response_event);
        if (status != FWK_SUCCESS)
            FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
    }

    ctx.current_event = NULL;

    fwk_interrupt_global_disable();
    fwk_list_push_tail(&ctx.free_event_queue, &event->slist_node);
    fwk_interrupt_global_enable();

    return;
}

static void process_isr(void)
{
    struct fwk_event *isr_event;

    fwk_interrupt_global_disable();
    isr_event = FWK_LIST_GET(fwk_list_pop_head(&ctx.isr_event_queue),
                             struct fwk_event, slist_node);
    fwk_interrupt_global_enable();

    FWK_LOG_TRACE(
        "[FWK] Get ISR event (%s: %s -> %s)\n",
        FWK_ID_STR(isr_event->id),
        FWK_ID_STR(isr_event->source_id),
        FWK_ID_STR(isr_event->target_id));

    fwk_list_push_tail(&ctx.event_queue, &isr_event->slist_node);
}

/*
 * Private interface functions
 */

int __fwk_thread_init(size_t event_count)
{
    struct fwk_event *event_table, *event;

    event_table = fwk_mm_calloc(event_count, sizeof(struct fwk_event));

    /* All the event structures are free to be used. */
    fwk_list_init(&ctx.free_event_queue);
    fwk_list_init(&ctx.event_queue);
    fwk_list_init(&ctx.isr_event_queue);

    for (event = event_table;
         event < (event_table + event_count);
         event++)
        fwk_list_push_tail(&ctx.free_event_queue, &event->slist_node);

    ctx.initialized = true;

    return FWK_SUCCESS;
}

noreturn void __fwk_thread_run(void)
{
    for (;;) {
        while (!fwk_list_is_empty(&ctx.event_queue))
            process_next_event();

        while (fwk_list_is_empty(&ctx.isr_event_queue)) {
            fwk_log_unbuffer();

            continue;
        }

        process_isr();
    }
}

struct __fwk_thread_ctx *__fwk_thread_get_ctx(void)
{
    return &ctx;
}

const struct fwk_event *__fwk_thread_get_current_event(void)
{
    return ctx.current_event;
}

#ifdef BUILD_HAS_NOTIFICATION
int __fwk_thread_put_notification(struct fwk_event *event)
{
    event->is_response = false;
    event->is_notification = true;

    return put_event(event);
}
#endif

/*
 * Public interface functions
 */

int fwk_thread_put_event(struct fwk_event *event)
{
    int status = FWK_E_PARAM;
    unsigned int interrupt;

    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (event == NULL)
        goto error;

    if ((fwk_interrupt_get_current(&interrupt) != FWK_SUCCESS) &&
        (ctx.current_event != NULL))
        event->source_id = ctx.current_event->target_id;
    else if (!fwk_module_is_valid_entity_id(event->source_id))
        goto error;

    if (!fwk_module_is_valid_entity_id(event->target_id) ||
        !fwk_module_is_valid_event_id(event->id))
        goto error;

    if (event->is_response) {
        if (fwk_id_get_module_idx(event->source_id) !=
            fwk_id_get_module_idx(event->id))
            goto error;
        if (event->response_requested)
            goto error;
    } else {
        if (fwk_id_get_module_idx(event->target_id) !=
            fwk_id_get_module_idx(event->id))
             goto error;
        if (event->is_notification)
            goto error;
    }

    return put_event(event);

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}
