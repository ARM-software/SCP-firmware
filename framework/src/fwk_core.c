/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Framework facilities.
 */

#include <internal/fwk_context.h>
#include <internal/fwk_core.h>
#include <internal/fwk_delayed_resp.h>
#include <internal/fwk_module.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <inttypes.h>
#include <stdbool.h>

static struct __fwk_ctx ctx;

static const char err_msg_line[] = "[FWK] Error %d in %s @%d";
static const char err_msg_func[] = "[FWK] Error %d in %s";

enum interrupt_states {
    UNKNOWN_STATE = 0,
    INTERRUPT_STATE = 1,
    NOT_INTERRUPT_STATE = 2,
};

/*
 * Static functions
 */

/*
 * Duplicate an event.
 *
 * \param event Pointer to the event to duplicate.
 * \param event_type Type of the event structure as defined in
 *     \c fwk_event_type
 *
 * \pre \p event must not be NULL
 *
 * \return The pointer to the duplicated event, NULL if the allocation to
 *      duplicate the event failed.
 */
static struct fwk_event *duplicate_event(
    void *event,
    enum fwk_event_type event_type)
{
    struct fwk_event *allocated_event = NULL;
    unsigned int flags;

    fwk_assert(event != NULL);

    flags = fwk_interrupt_global_disable();
    allocated_event = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.free_event_queue), struct fwk_event, slist_node);
    (void)fwk_interrupt_global_enable(flags);

    if (allocated_event == NULL) {
        FWK_LOG_CRIT(err_msg_func, FWK_E_NOMEM, __func__);
        fwk_unexpected();

        return NULL;
    }
    if (event_type == FWK_EVENT_TYPE_LIGHT) {
        struct fwk_event_light *light_event = (struct fwk_event_light *)event;
        allocated_event->id = light_event->id;
        allocated_event->source_id = light_event->source_id;
        allocated_event->target_id = light_event->target_id;
        allocated_event->is_notification = false;
        allocated_event->response_requested = light_event->response_requested;
        allocated_event->is_delayed_response = false;
        allocated_event->is_response = false;
    } else {
        *allocated_event = *((struct fwk_event *)event);
    }

    allocated_event->slist_node = (struct fwk_slist_node){ 0 };

    return allocated_event;
}

static int put_event(
    void *event,
    enum interrupt_states intr_state,
    enum fwk_event_type event_type)
{
    struct fwk_event *allocated_event;

    struct fwk_event *std_event = NULL;

    if (event_type == FWK_EVENT_TYPE_STD) {
        std_event = (struct fwk_event *)event;
    }

    if (std_event != NULL && std_event->is_delayed_response) {
        allocated_event = __fwk_search_delayed_response(
            std_event->source_id, std_event->cookie);
        if (allocated_event == NULL) {
            FWK_LOG_CRIT(err_msg_func, FWK_E_NOMEM, __func__);
            return FWK_E_PARAM;
        }

        fwk_list_remove(
            __fwk_get_delayed_response_list(std_event->source_id),
            &allocated_event->slist_node);

        (void)memcpy(
            allocated_event->params,
            std_event->params,
            sizeof(allocated_event->params));

    } else {
        allocated_event = duplicate_event(event, event_type);
        if (allocated_event == NULL) {
            return FWK_E_NOMEM;
        }
    }

    if (std_event != NULL) {
        allocated_event->cookie = ctx.event_cookie_counter++;
        std_event->cookie = allocated_event->cookie;
    }

    if (intr_state == UNKNOWN_STATE) {
        if (fwk_is_interrupt_context()) {
            intr_state = INTERRUPT_STATE;
        } else {
            intr_state = NOT_INTERRUPT_STATE;
        }
    }
    if (intr_state == NOT_INTERRUPT_STATE) {
        fwk_list_push_tail(&ctx.event_queue, &allocated_event->slist_node);
    } else {
        fwk_list_push_tail(&ctx.isr_event_queue, &allocated_event->slist_node);
    }

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
    FWK_LOG_DEBUG(
        "[FWK] Sent %" PRIu32 ": %s @ %s -> %s",
        std_event != NULL ? std_event->cookie : 0,
        FWK_ID_STR(allocated_event->id),
        FWK_ID_STR(allocated_event->source_id),
        FWK_ID_STR(allocated_event->target_id));
#endif

    return FWK_SUCCESS;
}

static void free_event(struct fwk_event *event)
{
    unsigned int flags;

    flags = fwk_interrupt_global_disable();
    fwk_list_push_tail(&ctx.free_event_queue, &event->slist_node);
    (void)fwk_interrupt_global_enable(flags);
}

static void process_next_event(void)
{
    int status;
    struct fwk_event *event, *allocated_event, async_response_event;
    const struct fwk_module *module;
    int (*process_event)(
        const struct fwk_event *event, struct fwk_event *resp_event);

    ctx.current_event = event = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.event_queue), struct fwk_event, slist_node);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
    FWK_LOG_DEBUG(
        "[FWK] Processing %" PRIu32 ": %s @ %s -> %s",
        event->cookie,
        FWK_ID_STR(event->id),
        FWK_ID_STR(event->source_id),
        FWK_ID_STR(event->target_id));
#endif

    module = fwk_module_get_ctx(event->target_id)->desc;
    process_event = event->is_notification ? module->process_notification :
                                             module->process_event;

    if (event->response_requested) {
        fwk_str_memset(&async_response_event, 0, sizeof(async_response_event));
        async_response_event = *event;
        async_response_event.source_id = event->target_id;
        async_response_event.target_id = event->source_id;
        async_response_event.is_delayed_response = false;

        status = process_event(event, &async_response_event);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
        }

        async_response_event.is_response = true;
        async_response_event.response_requested = false;
        if (!async_response_event.is_delayed_response) {
            (void)put_event(
                &async_response_event, UNKNOWN_STATE, FWK_EVENT_TYPE_STD);
        } else {
            allocated_event =
                duplicate_event(&async_response_event, FWK_EVENT_TYPE_STD);
            if (allocated_event != NULL) {
                fwk_list_push_tail(
                    __fwk_get_delayed_response_list(
                        async_response_event.source_id),
                    &allocated_event->slist_node);
            }
        }
    } else {
        status = process_event(event, &async_response_event);
        if ((status != FWK_SUCCESS) && (status != FWK_PENDING)) {
            FWK_LOG_CRIT(
                "[FWK] Process event (%s: %s -> %s) (%d)",
                FWK_ID_STR(event->id),
                FWK_ID_STR(event->source_id),
                FWK_ID_STR(event->target_id),
                status);
        }
    }

    ctx.current_event = NULL;
    free_event(event);
    return;
}

static bool process_isr(void)
{
    struct fwk_event *isr_event;
    unsigned int flags;

    flags = fwk_interrupt_global_disable();
    isr_event = FWK_LIST_GET(
        fwk_list_pop_head(&ctx.isr_event_queue), struct fwk_event, slist_node);
    (void)fwk_interrupt_global_enable(flags);

    if (isr_event == NULL) {
        return false;
    }

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
    FWK_LOG_DEBUG(
        "[FWK] Pulled ISR event (%s: %s -> %s)",
        FWK_ID_STR(isr_event->id),
        FWK_ID_STR(isr_event->source_id),
        FWK_ID_STR(isr_event->target_id));
#endif

    fwk_list_push_tail(&ctx.event_queue, &isr_event->slist_node);

    return true;
}

/*
 * Private interface functions
 */

int __fwk_init(size_t event_count)
{
    struct fwk_event *event_table, *event;

    event_table = fwk_mm_calloc(event_count, sizeof(struct fwk_event));

    /* All the event structures are free to be used. */
    fwk_list_init(&ctx.free_event_queue);
    fwk_list_init(&ctx.event_queue);
    fwk_list_init(&ctx.isr_event_queue);

    for (event = event_table; event < (event_table + event_count); event++) {
        fwk_list_push_tail(&ctx.free_event_queue, &event->slist_node);
    }

    ctx.initialized = true;

    return FWK_SUCCESS;
}

void fwk_process_event_queue(void)
{
    for (;;) {
        while (!fwk_list_is_empty(&ctx.event_queue)) {
            process_next_event();
        }

        if (!process_isr()) {
            break;
        }
    }
}

noreturn void __fwk_run_main_loop(void)
{
    for (;;) {
        fwk_process_event_queue();
        if (fwk_log_unbuffer() == FWK_SUCCESS) {
            fwk_arch_suspend();
        }
    }
}

struct __fwk_ctx *__fwk_get_ctx(void)
{
    return &ctx;
}

const struct fwk_event *__fwk_get_current_event(void)
{
    return ctx.current_event;
}

#ifdef BUILD_HAS_NOTIFICATION
int __fwk_put_notification(struct fwk_event *event)
{
    event->is_response = false;
    event->is_notification = true;

    return put_event(event, UNKNOWN_STATE, FWK_EVENT_TYPE_STD);
}
#endif

/*
 * Public interface functions
 */

int __fwk_put_event(struct fwk_event *event)
{
    int status = FWK_E_PARAM;
    enum interrupt_states intr_state;

#ifdef BUILD_MODE_DEBUG
    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (event == NULL) {
        goto error;
    }
#endif

    if (fwk_is_interrupt_context()) {
        intr_state = INTERRUPT_STATE;
    } else {
        intr_state = NOT_INTERRUPT_STATE;
    }

    if ((intr_state == NOT_INTERRUPT_STATE) && (ctx.current_event != NULL)) {
        event->source_id = ctx.current_event->target_id;
    } else if (
        !fwk_id_type_is_valid(event->source_id) ||
        !fwk_module_is_valid_entity_id(event->source_id)) {
        status = FWK_E_PARAM;
        goto error;
    }

#ifdef BUILD_MODE_DEBUG
    status = FWK_E_PARAM;
    if (event->is_notification) {
        if (!fwk_module_is_valid_notification_id(event->id)) {
            goto error;
        }
        if ((!event->is_response) || (event->response_requested)) {
            goto error;
        }
        if (fwk_id_get_module_idx(event->target_id) !=
            fwk_id_get_module_idx(event->id)) {
            goto error;
        }
    } else {
        if (!fwk_module_is_valid_event_id(event->id)) {
            goto error;
        }
        if (event->is_response) {
            if (fwk_id_get_module_idx(event->source_id) !=
                fwk_id_get_module_idx(event->id)) {
                goto error;
            }
            if (event->response_requested) {
                goto error;
            }
        } else {
            if (fwk_id_get_module_idx(event->target_id) !=
                fwk_id_get_module_idx(event->id)) {
                goto error;
            }
        }
    }
#endif

    return put_event(event, intr_state, FWK_EVENT_TYPE_STD);

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int __fwk_put_event_light(struct fwk_event_light *event)
{
    int status = FWK_E_PARAM;
    enum interrupt_states intr_state;

#ifdef BUILD_MODE_DEBUG
    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (event == NULL) {
        goto error;
    }
#endif

    if (fwk_is_interrupt_context()) {
        intr_state = INTERRUPT_STATE;
    } else {
        intr_state = NOT_INTERRUPT_STATE;
    }

    if ((intr_state == NOT_INTERRUPT_STATE) && (ctx.current_event != NULL)) {
        event->source_id = ctx.current_event->target_id;
    } else if (
        !fwk_id_type_is_valid(event->source_id) ||
        !fwk_module_is_valid_entity_id(event->source_id)) {
        status = FWK_E_PARAM;
        goto error;
    }

#ifdef BUILD_MODE_DEBUG
    status = FWK_E_PARAM;

    if (!fwk_module_is_valid_event_id(event->id)) {
        goto error;
    }

    if (fwk_id_get_module_idx(event->target_id) !=
        fwk_id_get_module_idx(event->id)) {
        goto error;
    }

#endif
    return put_event(event, intr_state, FWK_EVENT_TYPE_LIGHT);

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}
