/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmsis_os2.h>
#include <rtx_os.h>

#include <internal/fwk_id.h>
#include <internal/fwk_module.h>
#include <internal/fwk_multi_thread.h>
#include <internal/fwk_thread.h>
#include <internal/fwk_thread_delayed_resp.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_multi_thread.h>
#include <fwk_noreturn.h>
#include <fwk_slist.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if __has_include(<fmw_memory.h>)
#    include <fmw_memory.h>
#endif

#ifndef FIRMWARE_STACK_SIZE
#    define FIRMWARE_STACK_SIZE 1536
#endif

#define SIGNAL_ISR_EVENT 0x01
#define SIGNAL_EVENT_TO_PROCESS 0x02
#define SIGNAL_EVENT_PROCESSED 0x04
#define SIGNAL_NO_READY_THREAD 0x08
#define SIGNAL_CHECK_LOGS 0x10

static struct __fwk_multi_thread_ctx ctx;
static const char err_msg_line[] = "[FWK] Error %d @%d";
static const char err_msg_func[] = "[FWK] Error %d in %s";

/*
 * Static functions
 */

/*
 * Initialize the attributes of thread.
 *
 * \param[out] attr Thread's attributes.
 *
 * \retval FWK_SUCCESS The initialization succeeded.
 * \retval FWK_E_NOMEM A memory allocation failed.
 */
static int init_thread_attr(osThreadAttr_t *attr)
{
    attr->name = "";
    attr->attr_bits = osThreadDetached;
    attr->cb_size = osRtxThreadCbSize;
    attr->cb_mem = fwk_mm_calloc(1, attr->cb_size);

    attr->stack_size = FIRMWARE_STACK_SIZE;
    attr->stack_mem = fwk_mm_calloc(1, attr->stack_size);

    attr->priority = osPriorityNormal;

    return FWK_SUCCESS;
}

/*
 * Put back an event into the queue of free events.
 *
 * \param event Pointer to the event.
 *
 * \pre \p event must not be NULL
 */
static void free_event(struct fwk_event *event)
{
    fwk_interrupt_global_disable();
    fwk_list_push_tail(&ctx.event_free_queue, &event->slist_node);
    fwk_interrupt_global_enable();
}

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
    struct fwk_event *allocated_event;

    assert(event != NULL);

    fwk_interrupt_global_disable();
    allocated_event = FWK_LIST_GET(fwk_list_pop_head(&ctx.event_free_queue),
                                   struct fwk_event, slist_node);
    fwk_interrupt_global_enable();

    if (allocated_event != NULL) {
        *allocated_event = *event;

        allocated_event->slist_node = (struct fwk_slist_node) { 0 };

        return allocated_event;
    }

    assert(false);
    FWK_LOG_CRIT(err_msg_func, FWK_E_NOMEM, __func__);
    return NULL;
}

/*
 * Get the thread context of a given module or element.
 *
 * \param id Module or element identifier.
 *
 * \return Thread context, NULL if the identifier is not valid.
 */
static struct __fwk_thread_ctx *thread_get_ctx(fwk_id_t id)
{
    struct fwk_module_ctx *module_ctx;
    struct fwk_element_ctx *element_ctx;

    if (fwk_module_is_valid_element_id(id)) {
        element_ctx = __fwk_module_get_element_ctx(id);
        if (element_ctx->thread_ctx != NULL)
            return element_ctx->thread_ctx;
        else
            id = FWK_ID_MODULE(id.element.module_idx);
    }

    if (fwk_module_is_valid_module_id(id)) {
        module_ctx = __fwk_module_get_ctx(id);
        return (module_ctx->thread_ctx != NULL) ?
               module_ctx->thread_ctx : &ctx.common_thread_ctx;
    }

    return NULL;
}

/*
 * Put an event in the ISR event queue.
 *
 * This function is a sub-routine of the fwk_thread_put_event() interface
 * function.
 *
 * \param event Pointer to the ISR event to queue.
 *
 * \retval FWK_SUCCESS The event was put successfully.
 * \retval FWK_E_NOMEM The free event queue is empty.
 */
static int put_isr_event(struct fwk_event *event)
{
    struct fwk_event *allocated_event;
    uint32_t flags;

    allocated_event = duplicate_event(event);
    if (allocated_event == NULL)
        return FWK_E_NOMEM;

    /*
     * Assumption: there are no interrupt priorities, at least among interrupts
     * leading to the call of fwk_thread_put_event(). As a consequence the
     * access to the ISR queue of events is not protected here against
     * concurrent accesses, as this function should only be executed from within
     * an interrupt context.
     */
    fwk_list_push_tail(&ctx.event_isr_queue, &allocated_event->slist_node);
    if (ctx.waiting_for_isr_event) {
        flags = osThreadFlagsSet(ctx.common_thread_ctx.os_thread_id,
                                 SIGNAL_ISR_EVENT);
        if ((int32_t)flags < 0) {
            FWK_LOG_CRIT(err_msg_func, FWK_E_OS, __func__);
            return FWK_E_OS;
        }
        ctx.waiting_for_isr_event = false;
    }

    return FWK_SUCCESS;
}

/*
 * Check if an event is the wake-up event waited for by a thread.
 *
 * \param thread_ctx Pointer to the context of the waiting thread.
 * \param event Pointer to the event to be checked.
 *
 * \pre \p thread_ctx must not be NULL
 * \pre \p event must not be NULL
 *
 * \retval True The event is the wake-up event waited for by the thread.
 * \retval False The event is not the wake-up event waited for by the thread.
 */
static bool is_thread_wakeup_event(struct __fwk_thread_ctx *thread_ctx,
                                   struct fwk_event *event)
{
    if (!thread_ctx->waiting_event_processing_completion)
        return false;

    if (!event->is_response)
        return false;

    return thread_ctx->response_event->cookie == event->cookie;
}

/*
 * Put an event in a thread queue.
 *
 * This function is a sub-routine of the fwk_thread_put_event() and
 * fwk_thread_put_event_and_wait() interface functions. If the thread queue was
 * empty, the thread is added at the end of the list of threads having at least
 * one event pending in its event queue.
 *
 * \param thread_ctx Pointer to the context of the thread target of the event.
 * \param event Pointer to the event to queue.
 *
 * \retval FWK_SUCCESS The event was put successfully.
 * \retval FWK_E_PARAM The event source is not valid.
 */
static int put_event(struct __fwk_thread_ctx *target_thread_ctx,
                     struct fwk_event *event)
{
    int status = FWK_E_PARAM;
    struct fwk_event *allocated_event;
    bool is_empty;

    event->is_thread_wakeup_event = is_thread_wakeup_event(
        target_thread_ctx, event);

    if (event->is_delayed_response) {
        allocated_event = __fwk_thread_search_delayed_response(
            event->source_id, event->cookie);
        if (allocated_event == NULL)
            goto error;

        fwk_list_remove(
            __fwk_thread_get_delayed_response_list(event->source_id),
            &allocated_event->slist_node);

        memcpy(allocated_event->params, event->params,
               sizeof(allocated_event->params));
        allocated_event->is_thread_wakeup_event = event->is_thread_wakeup_event;
    } else {
        allocated_event = duplicate_event(event);
        if (allocated_event == NULL)
            return FWK_E_NOMEM;
    }

    allocated_event->cookie = event->cookie = ctx.event_cookie_counter++;

    if (allocated_event->is_thread_wakeup_event) {
        fwk_list_push_head(&target_thread_ctx->event_queue,
                           &allocated_event->slist_node);
        fwk_list_push_head(&ctx.thread_ready_queue,
                           &target_thread_ctx->slist_node);
    } else {
        is_empty = fwk_list_is_empty(&target_thread_ctx->event_queue);
        fwk_list_push_tail(&target_thread_ctx->event_queue,
                           &allocated_event->slist_node);

        if (is_empty &&
            (target_thread_ctx != ctx.current_thread_ctx) &&
            (!(target_thread_ctx->waiting_event_processing_completion)))
            fwk_list_push_tail(&ctx.thread_ready_queue,
                               &target_thread_ctx->slist_node);
    }

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

/*
 * Process event requiring a response
 *
 * This function is a sub-routine of process_next_thread_event().
 *
 * \param event Pointer to the current event.
 */
static void process_event_requiring_response(struct fwk_event *event)
{
    int status;
    const struct fwk_module *module;
    struct __fwk_thread_ctx *source_thread_ctx;
    struct fwk_event resp_event, *allocated_event;
    int (*process_event)(const struct fwk_event *event,
                         struct fwk_event *resp_event);

    module = __fwk_module_get_ctx(event->target_id)->desc;
    source_thread_ctx = thread_get_ctx(event->source_id);

    process_event = event->is_notification ?
        module->process_notification : module->process_event;

    resp_event = *event;
    resp_event.source_id = event->target_id;
    resp_event.target_id = event->source_id;
    resp_event.is_delayed_response = false;

    status = process_event(event, &resp_event);
    if (status != FWK_SUCCESS)
        FWK_LOG_CRIT(err_msg_line, status, __LINE__);

    resp_event.is_response = true;
    resp_event.response_requested = false;
    if (!resp_event.is_delayed_response)
        put_event(source_thread_ctx, &resp_event);
    else {
        allocated_event = duplicate_event(&resp_event);
        if (allocated_event != NULL) {
            fwk_list_push_tail(
                __fwk_thread_get_delayed_response_list(resp_event.source_id),
                &allocated_event->slist_node);
        }
    }
}

/*
 * Process the next event of a given thread.
 *
 * This function is a sub-routine of thread_function().
 *
 * \param thread_ctx Pointer to the context of the thread, the next event of
 *     which has to be processed.
 */
static void process_next_thread_event(struct __fwk_thread_ctx *thread_ctx)
{
    int status;
    struct fwk_event *event, async_resp_event;
    const struct fwk_module *module;

    /*
     * Extract the event from the thread event queue and update the pointer to
     * the currently active thread context and to the currently processed
     * event.
     */
    ctx.current_thread_ctx = thread_ctx;
    ctx.current_event = event =
        FWK_LIST_GET(fwk_list_pop_head(&thread_ctx->event_queue),
                     struct fwk_event, slist_node);
    assert(event != NULL);

    if (event->response_requested)
        process_event_requiring_response(event);
    else {
        module = __fwk_module_get_ctx(event->target_id)->desc;
        if (event->is_notification)
            status = module->process_notification(event, &async_resp_event);
        else
            status = module->process_event(event, &async_resp_event);
        if (status != FWK_SUCCESS)
            FWK_LOG_CRIT(err_msg_line, status, __LINE__);
    }

    /* No event currently processed, no thread currently active. */
    ctx.current_event = NULL;
    ctx.current_thread_ctx = NULL;

    free_event(event);

    if (!fwk_list_is_empty(&thread_ctx->event_queue))
        fwk_list_push_tail(&ctx.thread_ready_queue, &thread_ctx->slist_node);
}

/*
 * Launch the processing of the next event.
 *
 * \param current_thread_ctx Pointer to the context of the thread currently
 *     executed.
 *
 * \retval NULL There is no event to process for the time being.
 * \return Pointer to the context of the thread the next event will be
 *     processed within. If the thread is different from the one currently
 *     executing the function has raised the SIGNAL_EVENT_TO_PROCESS to the
 *     next thread for it to start executing when the current thread will go
 *     waiting.
 */
static struct __fwk_thread_ctx *launch_next_event_processing(
    struct __fwk_thread_ctx *current_thread_ctx)
{
    struct __fwk_thread_ctx *next_thread_ctx;
    uint32_t flags;
    struct fwk_event *event;

    while (!fwk_list_is_empty(&ctx.thread_ready_queue)) {
        next_thread_ctx = FWK_LIST_GET(
            fwk_list_pop_head(&ctx.thread_ready_queue),
            struct __fwk_thread_ctx, slist_node);

        if (next_thread_ctx == current_thread_ctx)
            return current_thread_ctx;

        event = FWK_LIST_GET(fwk_list_head(&next_thread_ctx->event_queue),
                             struct fwk_event, slist_node);

        if (event->is_thread_wakeup_event) {
            *next_thread_ctx->response_event = *event;
            flags = osThreadFlagsSet(next_thread_ctx->os_thread_id,
                                     SIGNAL_EVENT_PROCESSED);
            if ((int32_t)flags >= 0) {
                event = FWK_LIST_GET(fwk_list_pop_head(
                    &next_thread_ctx->event_queue), struct fwk_event,
                    slist_node);
                free_event(event);
                return next_thread_ctx;
            }
        } else {
            flags = osThreadFlagsSet(next_thread_ctx->os_thread_id,
                                     SIGNAL_EVENT_TO_PROCESS);
            if ((int32_t)flags >= 0)
                return next_thread_ctx;
        }

        FWK_LOG_CRIT(err_msg_line, FWK_E_OS, __LINE__);
        event = FWK_LIST_GET(fwk_list_pop_head(&next_thread_ctx->event_queue),
                             struct fwk_event, slist_node);
        free_event(event);

        if (!fwk_list_is_empty(&next_thread_ctx->event_queue)) {
            fwk_list_push_tail(&ctx.thread_ready_queue,
                               &next_thread_ctx->slist_node);
        }
    }

    return NULL;
}

static void thread_function(struct __fwk_thread_ctx *thread_ctx,
                            struct __fwk_thread_ctx *next_thread_ctx)
{
    uint32_t flags;
    uint32_t signals;

    signals = (thread_ctx == &ctx.common_thread_ctx) ?
              (SIGNAL_EVENT_TO_PROCESS | SIGNAL_NO_READY_THREAD) :
              SIGNAL_EVENT_TO_PROCESS;

    for (;;) {
        /*
         * If the thread which the next event has to be processed within is the
         * present thread, proceed straight to the processing of the next event.
         * Otherwise, wait for a signal:
         * 1) just the event to process signal for specific threads
         * 2) event to process and no ready threads for the common thread
         */
        if (thread_ctx != next_thread_ctx) {
            flags = osThreadFlagsWait(signals, osFlagsWaitAny, osWaitForever);
            /* If there is more than one flag raised or not an expected one */
            if ((flags & (flags - 1)) || (!(flags & signals))) {
                FWK_LOG_CRIT(err_msg_line, FWK_E_OS, __LINE__);
                continue;
            }
            if (flags & SIGNAL_NO_READY_THREAD)
                return;
        }

       /* Process the first event of the thread event queue */
        next_thread_ctx = NULL;
        process_next_thread_event(thread_ctx);
        next_thread_ctx = launch_next_event_processing(thread_ctx);

        /*
         * There is an event to process now. The thread it has to be processed
         * within is the thread of context 'next_thread_ctx'. If it has to be
         * processed by the current thread (next_thread_ctx == thread_ctx), loop
         * to process it. If the event has to be processed by another thread
         * (next_thread_ctx != thread_ctx), the present thread has successfully
         * signalled it to the other thread, loop to wait for a wake-up signal.
         */
        if (next_thread_ctx != NULL)
            continue;

        /*
         * No event to process for the time being. If the current thread is the
         * common one just return to the __fwk_thread_run() function to get the
         * next ISR event. Otherwise, signal the common thread with a
         * SIGNAL_NO_READY_THREAD signal that it has to get the next ISR event.
         */
        if (thread_ctx == &ctx.common_thread_ctx)
            return;
        flags = osThreadFlagsSet(
            ctx.common_thread_ctx.os_thread_id, SIGNAL_NO_READY_THREAD);
        if ((int32_t)flags < 0)
            FWK_LOG_CRIT(err_msg_line, FWK_E_OS, __LINE__);

        /* Let the logging thread know we might have messages to process */
        flags = osThreadFlagsSet(ctx.log_thread_id, SIGNAL_CHECK_LOGS);
        if ((int32_t)flags < 0)
            FWK_LOG_CRIT(err_msg_line, FWK_E_OS, __LINE__);
    }
}

static void specific_thread_function(void *arg)
{
    assert(arg != NULL);

    thread_function((struct __fwk_thread_ctx *)arg, NULL);
}

static void get_next_isr_event(void)
{
    uint32_t flags;
    struct fwk_event *isr_event;
    struct __fwk_thread_ctx *target_thread_ctx;

    for (;;) {
        fwk_interrupt_global_disable();
        if (fwk_list_is_empty(&ctx.event_isr_queue)) {
            /* Wait for an ISR event. */
            ctx.waiting_for_isr_event = true;
            fwk_interrupt_global_enable();
            flags = osThreadFlagsWait(
                SIGNAL_ISR_EVENT, osFlagsWaitAll, osWaitForever);
            if (flags != SIGNAL_ISR_EVENT)
                FWK_LOG_CRIT(err_msg_line, FWK_E_OS, __LINE__);
            continue;
        }

        isr_event = FWK_LIST_GET(fwk_list_pop_head(&ctx.event_isr_queue),
                                 struct fwk_event, slist_node);
        fwk_interrupt_global_enable();

        assert(isr_event != NULL);

        FWK_LOG_TRACE(
            "[FWK] Get ISR event (%s: %s -> %s)\n",
            FWK_ID_STR(isr_event->id),
            FWK_ID_STR(isr_event->source_id),
            FWK_ID_STR(isr_event->target_id));

        target_thread_ctx = thread_get_ctx(isr_event->target_id);

        isr_event->is_thread_wakeup_event = is_thread_wakeup_event(
            target_thread_ctx, isr_event);
        isr_event->cookie = ctx.event_cookie_counter++;

        if (isr_event->is_thread_wakeup_event) {
            fwk_list_push_head(&target_thread_ctx->event_queue,
                               &isr_event->slist_node);
        } else {
            fwk_list_push_tail(&target_thread_ctx->event_queue,
                               &isr_event->slist_node);
        }

        if (!(target_thread_ctx->waiting_event_processing_completion) ||
            isr_event->is_thread_wakeup_event) {
            fwk_list_push_head(&ctx.thread_ready_queue,
                               &target_thread_ctx->slist_node);
            break;
        }
    }
}

static void common_thread_function(void *arg)
{
    struct __fwk_thread_ctx *next_thread_ctx;

    ctx.running = true;

    for (;;) {
        next_thread_ctx = launch_next_event_processing(&ctx.common_thread_ctx);

        if (next_thread_ctx != NULL)
            thread_function(&ctx.common_thread_ctx, next_thread_ctx);

        get_next_isr_event();
    }
}

static void logging_thread(void *arg)
{
    while (true) {
        int status;

        (void)osThreadFlagsWait(
            SIGNAL_CHECK_LOGS, osFlagsNoClear, osWaitForever);

        /*
         * At this point we've received a signal from one of the other threads
         * that there might be log messages we need to process. Because logging
         * is a low-priority task, we yield as soon as we've printed a character
         * in order to maintain lower latencies elsewhere.
         *
         * We will only clear the signal once we have emptied out the log
         * buffer (at which point we will enter an idle state).
         */

        status = fwk_log_unbuffer();
        if (status == FWK_SUCCESS)
            osThreadFlagsClear(SIGNAL_CHECK_LOGS);
        else if (status != FWK_PENDING)
            FWK_LOG_WARN("[FWK] Warning: unable to unbuffer logged message");
    }
}

/*
 * Private interface functions
 */

int __fwk_thread_init(size_t event_count)
{
    int status;
    struct fwk_event *event_table, *event_table_end, *event;
    osThreadAttr_t thread_attr;

    fwk_interrupt_global_enable();
    status = osKernelInitialize();
    if (status != osOK) {
        status = FWK_E_OS;
        goto error;
    }

    event_table = fwk_mm_calloc(event_count, sizeof(struct fwk_event));

    /* All the event structures are free to be used. */
    fwk_list_init(&ctx.event_free_queue);
    fwk_list_init(&(ctx.thread_ready_queue));
    fwk_list_init(&(ctx.event_isr_queue));
    fwk_list_init(&(ctx.common_thread_ctx.event_queue));
    for (event = event_table, event_table_end = event_table + event_count;
         event < event_table_end; event++)
        fwk_list_push_tail(&ctx.event_free_queue,
                           &event->slist_node);

    status = init_thread_attr(&thread_attr);
    if (status != FWK_SUCCESS)
        goto error;

    ctx.common_thread_ctx.os_thread_id = osThreadNew(common_thread_function,
        &ctx.common_thread_ctx, &thread_attr);
    if (ctx.common_thread_ctx.os_thread_id == NULL) {
        status = FWK_E_OS;
        goto error;
    }

    /* Initialize the logging thread */

    status = init_thread_attr(&thread_attr);
    if (status != FWK_SUCCESS)
        goto error;

    thread_attr.priority = osPriorityLow;

    ctx.log_thread_id = osThreadNew(logging_thread, NULL, &thread_attr);
    if (ctx.log_thread_id == NULL) {
        status = FWK_E_OS;

        goto error;
    }

    ctx.initialized = true;

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

struct __fwk_multi_thread_ctx *__fwk_multi_thread_get_ctx(void)
{
   return &ctx;
}

noreturn void __fwk_thread_run(void)
{
    osKernelStart();

    while (true)
        continue;
}

const struct fwk_event *__fwk_thread_get_current_event(void)
{
    return ctx.current_event;
}

#ifdef BUILD_HAS_NOTIFICATION
int __fwk_thread_put_notification(struct fwk_event *event)
{
    unsigned int interrupt;

    event->is_response = false;
    event->is_notification = true;
    event->is_delayed_response = false;

    /* Call from a thread */
    if (fwk_interrupt_get_current(&interrupt) != FWK_SUCCESS)
        return put_event(thread_get_ctx(event->target_id), event);

    /* Call from an ISR */
    if (!fwk_module_is_valid_entity_id(event->source_id)) {
        FWK_LOG_CRIT(err_msg_func, FWK_E_PARAM, __func__);
        return FWK_E_PARAM;
    }

    return put_isr_event(event);
}
#endif

/*
 * Public interface functions
 */
int fwk_thread_create(fwk_id_t id)
{
    int status;
    struct __fwk_thread_ctx **p_thread_ctx, *thread_ctx;
    osThreadAttr_t thread_attr;

    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (fwk_module_is_valid_element_id(id))
        p_thread_ctx = &__fwk_module_get_element_ctx(id)->thread_ctx;
    else if (fwk_module_is_valid_module_id(id))
        p_thread_ctx = &__fwk_module_get_ctx(id)->thread_ctx;
    else {
        status = FWK_E_PARAM;
        goto error;
    }

    status = init_thread_attr(&thread_attr);
    if (status != FWK_SUCCESS)
        goto error;

    if ((ctx.running) || (*p_thread_ctx != NULL)) {
        status = FWK_E_STATE;
        goto error;
    }

    thread_ctx = fwk_mm_calloc(1, sizeof(struct __fwk_thread_ctx));
    if (thread_ctx == NULL) {
        status = FWK_E_NOMEM;
        goto error;
    }

    fwk_list_init(&thread_ctx->event_queue);
    thread_ctx->id = id;
    thread_ctx->os_thread_id = osThreadNew(specific_thread_function, thread_ctx,
                                           &thread_attr);

    if (thread_ctx->os_thread_id == NULL) {
        status = FWK_E_OS;
        goto error;
    }

    *p_thread_ctx = thread_ctx;

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int fwk_thread_put_event(struct fwk_event *event)
{
    int status = FWK_E_PARAM;
    struct __fwk_thread_ctx *thread_ctx;
    unsigned int interrupt;

    if (!ctx.initialized) {
        status = FWK_E_INIT;
        goto error;
    }

    if (event == NULL)
        goto error;

    thread_ctx = thread_get_ctx(event->target_id);
    if (thread_ctx == NULL)
        goto error;

    if ((fwk_interrupt_get_current(&interrupt) != FWK_SUCCESS) &&
        (ctx.current_event != NULL))
        event->source_id = ctx.current_event->target_id;
    else {
        if (!fwk_module_is_valid_entity_id(event->source_id))
            goto error;
    }

    if (event->is_notification) {
        if (!fwk_module_is_valid_notification_id(event->id))
            goto error;
        if ((!event->is_response) || (event->response_requested))
            goto error;
        if (fwk_id_get_module_idx(event->target_id) !=
            fwk_id_get_module_idx(event->id))
             goto error;
    } else {
        if (!fwk_module_is_valid_event_id(event->id))
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
        }
    }

    /* Call from a thread */
    if (fwk_interrupt_get_current(&interrupt) != FWK_SUCCESS) {
        event->is_delayed_response = event->is_response;
        return put_event(thread_ctx, event);
    }

    /* Call from an ISR */
    return put_isr_event(event);

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}

int fwk_thread_put_event_and_wait(struct fwk_event *event,
                                  struct fwk_event *resp_event)
{
    int status = FWK_E_PARAM;
    struct __fwk_thread_ctx *target_thread_ctx;
    unsigned int interrupt;
    struct __fwk_thread_ctx *calling_thread_ctx;
    struct fwk_event *processed_event;
    uint32_t flags;

    if (!ctx.running) {
        status = FWK_E_STATE;
        goto error;
    }

    if ((event == NULL) || (resp_event == NULL))
        goto error;

    target_thread_ctx = thread_get_ctx(event->target_id);
    if (target_thread_ctx == NULL)
        goto error;

    if (!fwk_module_is_valid_event_id(event->id))
        goto error;

    if ((fwk_interrupt_get_current(&interrupt) == FWK_SUCCESS) ||
        (ctx.current_thread_ctx == &ctx.common_thread_ctx) ||
        (target_thread_ctx == ctx.current_thread_ctx)) {
        status = FWK_E_ACCESS;
        goto error;
    }

    if (ctx.current_event != NULL)
        event->source_id = ctx.current_event->target_id;
    else if (!fwk_module_is_valid_entity_id(event->source_id))
            goto error;

    event->is_response = false;
    event->is_delayed_response = false;
    event->response_requested = true;
    event->is_notification = false;
    status = put_event(target_thread_ctx, event);
    if (status != FWK_SUCCESS)
        return status;

    resp_event->cookie = event->cookie;
    ctx.current_thread_ctx->response_event = resp_event;
    ctx.current_thread_ctx->waiting_event_processing_completion = true;

    /* Save the current thread and the current event */
    calling_thread_ctx = ctx.current_thread_ctx;
    processed_event = ctx.current_event;

    /*
     * Launch the processing of the next event if possible.
     */
    launch_next_event_processing(ctx.current_thread_ctx);

    /* Wait the completion of 'processed_event' */
    flags = osThreadFlagsWait(SIGNAL_EVENT_PROCESSED,
                              osFlagsWaitAll, osWaitForever);

    calling_thread_ctx->response_event = NULL;
    calling_thread_ctx->waiting_event_processing_completion = false;

    if (flags != SIGNAL_EVENT_PROCESSED) {
        status = FWK_E_OS;
        goto error;
    }

    /* Restore the context of the current thread and the current event */
    ctx.current_thread_ctx = calling_thread_ctx;
    ctx.current_event = processed_event;

    return FWK_SUCCESS;

error:
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}
