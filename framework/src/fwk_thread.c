/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Single-thread facilities.
 */

#include <stdbool.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_errno.h>
#include <fwk_host.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <internal/fwk_module.h>
#include <internal/fwk_notification.h>
#include <internal/fwk_single_thread.h>
#include <internal/fwk_thread.h>

static struct __fwk_thread_ctx ctx;

#ifdef BUILD_HOST
static const char err_msg_line[] = "[THR] Error %d in %s @%d\n";
static const char err_msg_func[] = "[THR] Error %d in %s\n";
#endif

/*
 * Static functions
 */

static int put_event(struct fwk_event *event)
{
    struct fwk_event *free_event;
    unsigned int interrupt;

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx.free_event_queue),
        struct fwk_event, slist_node);

    if (free_event == NULL) {
        FWK_HOST_PRINT(err_msg_func, FWK_E_NOMEM, __func__);
        assert(false);
        return FWK_E_NOMEM;
    }

    *free_event = *event;

    if (fwk_interrupt_get_current(&interrupt) != FWK_SUCCESS)
        fwk_list_push_tail(&ctx.event_queue, &free_event->slist_node);
    else
        fwk_list_push_tail(&ctx.isr_event_queue, &free_event->slist_node);

    return FWK_SUCCESS;
}

static void process_next_event(void)
{
    int status;
    struct fwk_event *event, async_response_event = {0};
    const struct fwk_module *module;
    int (*process_event)(const struct fwk_event *event,
                         struct fwk_event *resp_event);


    ctx.current_event = event = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.event_queue), struct fwk_event, slist_node);

    FWK_HOST_PRINT("[THR] Get event (%s,%s,%s)\n",
                   FWK_ID_STR(event->source_id), FWK_ID_STR(event->target_id),
                   FWK_ID_STR(event->id));

    module = __fwk_module_get_ctx(event->target_id)->desc;
    process_event = event->is_notification ? module->process_notification :
                    module->process_event;

    if (event->response_requested) {
        async_response_event.source_id = event->target_id;
        async_response_event.target_id = event->source_id;
        async_response_event.id = event->id;
        memcpy(&async_response_event.params, &event->params,
               sizeof(async_response_event.params));

        status = process_event(event, &async_response_event);
        if (status != FWK_SUCCESS)
            FWK_HOST_PRINT(err_msg_line, status, __func__, __LINE__);

        async_response_event.is_response = true;
        async_response_event.response_requested = false;
        async_response_event.is_notification = event->is_notification;
        if (!async_response_event.is_delayed_response)
            put_event(&async_response_event);
    } else {
        status = process_event(event, &async_response_event);
        if (status != FWK_SUCCESS)
            FWK_HOST_PRINT(err_msg_line, status, __func__, __LINE__);
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

    FWK_HOST_PRINT("[THR] Get ISR event (%s,%s,%s)\n",
                   FWK_ID_STR(isr_event->source_id),
                   FWK_ID_STR(isr_event->target_id),
                   FWK_ID_STR(isr_event->id));

    fwk_list_push_tail(&ctx.event_queue, &isr_event->slist_node);
}

/*
 * Private interface functions
 */

int __fwk_thread_init(size_t event_count)
{
    int status;
    struct fwk_event *event_table, *event;

    event_table = fwk_mm_calloc(event_count, sizeof(struct fwk_event));
    if (event_table == NULL) {
        status = FWK_E_NOMEM;
        goto error;
    }

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

error:
    FWK_HOST_PRINT(err_msg_func, status, __func__);
    return status;
}

noreturn void __fwk_thread_run(void)
{
    for (;;) {
        while (!fwk_list_is_empty(&ctx.event_queue))
            process_next_event();

        while (fwk_list_is_empty(&ctx.isr_event_queue))
            continue;

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
    FWK_HOST_PRINT(err_msg_func, status, __func__);
    return status;
}
