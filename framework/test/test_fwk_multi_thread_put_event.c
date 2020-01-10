/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_id.h>
#include <internal/fwk_module.h>
#include <internal/fwk_multi_thread.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_event.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ELEM_THREAD_ID      10
#define MODULE_THREAD_ID    11
#define COMMON_THREAD_ID    12

#define SIGNAL_ISR_EVENT 0x01
#define SIGNAL_EVENT_TO_PROCESS 0x02
#define SIGNAL_EVENT_PROCESSED 0x04
#define SIGNAL_NO_READY_THREAD 0x08

static jmp_buf test_context;
struct fwk_event event[4];
struct fwk_event notification;
struct __fwk_multi_thread_ctx *ctx;

/* Mock module and element */
static struct __fwk_thread_ctx fake_thread_element_ctx;
static struct fwk_element fake_element;
static struct fwk_element_ctx fake_element_ctx;
static struct __fwk_thread_ctx fake_thread_module_ctx;
static struct fwk_module fake_module;
static struct fwk_module_ctx fake_module_ctx;
static struct fwk_event fake_module_response_event;

static int fwk_thread_put_event_and_wait_return_val;
static bool process_event_call_thread_put_event_and_wait;
static int process_event(const struct fwk_event *evt,
                         struct fwk_event *response_event)
{
    (void) evt;
    (void) response_event;
    if (process_event_call_thread_put_event_and_wait) {
        fwk_thread_put_event_and_wait_return_val =
            fwk_thread_put_event_and_wait(&event[2], &event[3]);
    }
    return FWK_SUCCESS;
}

/* Wrapped OS functions */

osStatus_t __wrap_osKernelStart(void)
{
    return osOK;
}

osStatus_t __wrap_osKernelInitialize(void)
{
    return osOK;
}
/*
 * Indicate the number of function calls before exiting the non-returning
 * functions
 */
static unsigned int osThreadFlagsWait_break;
static unsigned int osThreadFlagsWait_count_call;
static uint32_t osThreadFlagsWait_return_val[3];
static uint32_t osThreadFlagsWait_param_flags[3];
static uint32_t osThreadFlagsWait_param_options[3];
uint32_t __wrap_osThreadFlagsWait(uint32_t flags, uint32_t options,
                                  uint32_t timeout)
{
    (void) timeout;
    osThreadFlagsWait_param_options[osThreadFlagsWait_count_call] = options;
    osThreadFlagsWait_param_flags[osThreadFlagsWait_count_call] = flags;
    if (osThreadFlagsWait_count_call == osThreadFlagsWait_break) {
        osThreadFlagsWait_count_call = 0;
        longjmp(test_context, !FWK_SUCCESS);
    } else
        return osThreadFlagsWait_return_val[osThreadFlagsWait_count_call++];
}

uint32_t __wrap_osThreadFlagsClear(uint32_t flags)
{
    (void)flags;
    return 0;
}

static uint32_t osThreadFlagsSet_return_val;
static osThreadId_t osThreadFlagsSet_param_thread_id[3];
static uint32_t osThreadFlagsSet_param_flags[3];
static unsigned int osThreadFlagsSet_count_call;
uint32_t __wrap_osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    osThreadFlagsSet_param_thread_id[osThreadFlagsSet_count_call] = thread_id;
    osThreadFlagsSet_param_flags[osThreadFlagsSet_count_call] = flags;
    osThreadFlagsSet_count_call++;
    return osThreadFlagsSet_return_val;
}

static osThreadFunc_t common_thread_function;
static osThreadFunc_t specific_thread_function;
static unsigned int osThreadNew_count_call;
osThreadId_t __wrap_osThreadNew(osThreadFunc_t func, void *argument,
                                const osThreadAttr_t *attr)
{
    osThreadId_t id;
    if (osThreadNew_count_call == 0) {
        common_thread_function = func;
        id = (osThreadId_t) COMMON_THREAD_ID;
    } else {
        specific_thread_function = func;
        id = (osThreadId_t) ELEM_THREAD_ID;
    }
    osThreadNew_count_call++;

    return id;
}

static int fwk_interrupt_get_current_return_val;
int __wrap_fwk_interrupt_get_current(unsigned int *interrupt)
{
    (void)interrupt;
    return fwk_interrupt_get_current_return_val;
}

/* Wrapped framework functions */
static void *fwk_mm_calloc_val;
void *__wrap_fwk_mm_calloc(size_t num, size_t size)
{
    fwk_mm_calloc_val = (void *)calloc(num, size);
    return fwk_mm_calloc_val;
}

int __wrap_fwk_interrupt_global_disable(void)
{
    return 0;
}

int __wrap_fwk_interrupt_global_enable(void)
{
    return 0;
}

bool __wrap_fwk_module_is_valid_element_id(fwk_id_t id)
{
    return false;
}

static bool fwk_module_is_valid_module_id_return_val;
bool __wrap_fwk_module_is_valid_module_id(fwk_id_t id)
{
    return fwk_module_is_valid_module_id_return_val;
}

static bool fwk_module_is_valid_entity_id_return_val;
bool __wrap_fwk_module_is_valid_entity_id(fwk_id_t id)
{
    return fwk_module_is_valid_entity_id_return_val;
}

static bool fwk_module_is_valid_event_id_return_val;
bool __wrap_fwk_module_is_valid_event_id(fwk_id_t id)
{
    return fwk_module_is_valid_event_id_return_val;
}

static bool fwk_module_is_valid_notification_id_return_val;
bool __wrap_fwk_module_is_valid_notification_id(fwk_id_t id)
{
    return fwk_module_is_valid_notification_id_return_val;
}

struct fwk_element_ctx *__wrap___fwk_module_get_element_ctx(fwk_id_t id)
{
    (void) id;
    return &fake_element_ctx;
}

struct fwk_module_ctx *__wrap___fwk_module_get_ctx(fwk_id_t id)
{
    (void) id;
    return &fake_module_ctx;
}

int __wrap___fwk_module_get_state(fwk_id_t id, enum fwk_module_state *state)
{
    (void) id;
    *state = FWK_MODULE_STATE_INITIALIZED;
    return FWK_SUCCESS;
}

static int test_suite_setup(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    status = __fwk_thread_init(2);
    if (status != FWK_SUCCESS)
        return status;

    fwk_module_is_valid_module_id_return_val = true;
    status = fwk_thread_create(FWK_ID_MODULE(0x1));
    if (status != FWK_SUCCESS)
        return status;

    ctx = __fwk_multi_thread_get_ctx();

    return FWK_SUCCESS;
}

static void test_case_setup(void)
{
    ctx->waiting_for_isr_event = false;
    ctx->running = false;
    ctx->current_thread_ctx = NULL;
    ctx->event_cookie_counter = 0;

    memset(event, 0, sizeof(event));

    event[0].source_id = FWK_ID_MODULE(0x1);
    event[0].target_id = FWK_ID_MODULE(0x2);
    event[0].cookie = 0;
    event[0].is_response = false;
    event[0].response_requested = false;
    event[0].id = FWK_ID_EVENT(2, 7);

    event[1].source_id = FWK_ID_MODULE(0x3);
    event[1].target_id = FWK_ID_MODULE(0x4);
    event[1].cookie = 0;
    event[1].is_response = false;
    event[1].response_requested = false;
    event[1].id = FWK_ID_EVENT(4, 8);

    event[2].source_id = FWK_ID_MODULE(0x5);
    event[2].target_id = FWK_ID_MODULE(0x6);
    event[2].cookie = 0;
    event[2].is_response = false;
    event[2].response_requested = true;
    event[2].id = FWK_ID_EVENT(6, 9);

    event[3].source_id = FWK_ID_MODULE(0x7);
    event[3].target_id = FWK_ID_MODULE(0x8);
    event[3].cookie = 0;
    event[3].is_response = false;
    event[3].response_requested = false;
    event[3].id = FWK_ID_EVENT(8, 10);

    memset(&notification, 0, sizeof(notification));

    notification.source_id = FWK_ID_MODULE(0x7);
    notification.target_id = FWK_ID_MODULE(0x8);
    notification.cookie = 0;
    notification.is_notification = true;
    notification.is_response = true;
    notification.response_requested = false;
    notification.id = FWK_ID_NOTIFICATION(8, 10);

    fwk_interrupt_get_current_return_val = FWK_SUCCESS;
    osThreadFlagsSet_return_val = 0;
    osThreadFlagsSet_count_call = 0;

    osThreadFlagsWait_break = 3;
    osThreadFlagsWait_return_val[0] = 1;
    osThreadFlagsWait_return_val[1] = 1;
    osThreadFlagsWait_return_val[2] = 1;
    osThreadFlagsWait_count_call = 0;

    memset(&fake_module_ctx, 0, sizeof(fake_module_ctx));
    memset(&fake_thread_module_ctx, 0, sizeof(fake_thread_module_ctx));
    memset(&fake_thread_element_ctx, 0, sizeof(fake_thread_element_ctx));

    fake_thread_element_ctx.os_thread_id = (osThreadId_t)ELEM_THREAD_ID;
    fake_thread_element_ctx.waiting_event_processing_completion = false;
    fake_thread_module_ctx.os_thread_id = (osThreadId_t)MODULE_THREAD_ID;
    fake_thread_module_ctx.waiting_event_processing_completion = false;
    fake_module.process_event = process_event;

    fake_module_ctx.desc = &fake_module;
    fake_module_ctx.thread_ctx = &fake_thread_module_ctx;
    fake_module_ctx.thread_ctx->response_event = NULL;

    fake_module_response_event = (struct fwk_event){};

    fake_element_ctx.desc = &fake_element;
    fake_element_ctx.thread_ctx = &fake_thread_element_ctx;

    fwk_module_is_valid_module_id_return_val = true;
    fwk_module_is_valid_entity_id_return_val = true;
    fwk_module_is_valid_event_id_return_val = true;
    fwk_module_is_valid_notification_id_return_val = true;

    process_event_call_thread_put_event_and_wait = false;
    fwk_thread_put_event_and_wait_return_val = FWK_SUCCESS;

    fwk_list_init(&ctx->event_free_queue);
    fwk_list_init(&ctx->thread_ready_queue);
    fwk_list_init(&ctx->event_isr_queue);
    fwk_list_init(&ctx->common_thread_ctx.event_queue);
    fwk_list_init(&fake_thread_module_ctx.event_queue);
    fwk_list_init(&fake_thread_element_ctx.event_queue);
    fwk_list_init(&fake_module_ctx.delayed_response_list);
    fwk_list_init(&fake_element_ctx.delayed_response_list);
}

static void test_put_event_ctx_not_initialized(void)
{
    int status;

    ctx->initialized = false;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_INIT);

    ctx->initialized = true;
}

static void test_put_event_invalid_event(void)
{
    int status;

    status = fwk_thread_put_event(NULL);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_invalid_thread_context(void)
{
    int status;

    /* Invalid thread context */
    fwk_module_is_valid_module_id_return_val = false;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_incompatible_target_event(void)
{
    int status;

    event[0].target_id = FWK_ID_MODULE(0x3);

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_incompatible_target_notification(void)
{
    int status;

    notification.target_id = FWK_ID_MODULE(0x7);

    status = fwk_thread_put_event(&notification);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_call_from_a_thread(void)
{
    int status;

    /* Call from a thread */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fwk_interrupt_get_current_return_val = FWK_E_STATE;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);

    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
    assert(osThreadFlagsSet_count_call == 0);
    assert(ctx->event_cookie_counter == 1);
}

static void test_put_event_function_failed(void)
{
    int status;

    /* The internal put event function failed */
    fwk_list_push_tail(&ctx->event_free_queue, &event[2].slist_node);
    ctx->current_event = NULL;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_module_is_valid_entity_id_return_val = false;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_invalid_entity_id(void)
{
    int status;
    /* Call from ISR but entity ID invalid */
    fwk_module_is_valid_entity_id_return_val = false;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_invalid_event_id(void)
{
    int status;
    /* Call from ISR but event ID invalid */
    fwk_module_is_valid_event_id_return_val = false;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_invalid_notification_id(void)
{
    int status;
    /* Call from ISR but notification ID invalid */
    fwk_module_is_valid_notification_id_return_val = false;

    status = fwk_thread_put_event(&notification);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_notification_invalid_response_flag(void)
{
    int status;
    notification.is_response = false;

    status = fwk_thread_put_event(&notification);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_notification_invalid_response_requested_flag(void)
{
    int status;
    notification.response_requested = true;

    status = fwk_thread_put_event(&notification);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_one_isr_event(void)
{
    int status;

    /* Only one ISR event */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    ctx->waiting_for_isr_event = true;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(ctx->event_isr_queue.tail == &event[1].slist_node);
    assert(osThreadFlagsSet_count_call == 1);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)COMMON_THREAD_ID);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_set_flags_failed(void)
{
    int status;

    /* One ISR event but flag set failed */
    ctx->waiting_for_isr_event = true;
    osThreadFlagsSet_return_val = -1;
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_OS);

    assert(ctx->event_isr_queue.tail == &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(osThreadFlagsSet_count_call == 1);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)COMMON_THREAD_ID);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_several_isr_events(void)
{
    int status;

    /* Several ISR events in the ISR event queue */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fwk_list_push_tail(&ctx->event_isr_queue, &event[2].slist_node);

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);

    assert(ctx->event_isr_queue.tail == &event[1].slist_node);
    assert(ctx->event_isr_queue.head == &event[2].slist_node);
    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(osThreadFlagsSet_count_call == 0);
    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_to_waiting_thread_not_delayed_response(void)
{
    int status;

    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fake_thread_module_ctx.waiting_event_processing_completion = true;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
    assert(osThreadFlagsSet_count_call == 0);
}

static void test_put_event_to_waiting_thread_not_wakeup_event(void)
{
    int status;

    fwk_list_push_tail(&fake_module_ctx.delayed_response_list,
                       &event[1].slist_node);
    fake_thread_module_ctx.waiting_event_processing_completion = true;
    event[0].source_id = FWK_ID_MODULE(0x2);
    event[0].target_id = FWK_ID_MODULE(0x1);
    event[0].is_response = true;
    event[0].cookie = 1;
    event[1].cookie = 1;
    fake_module_response_event.cookie = 2;
    fake_module_ctx.thread_ctx->response_event = &fake_module_response_event;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
    assert(osThreadFlagsSet_count_call == 0);
}

static void test_put_event_to_waiting_thread_wakeup_event(void)
{
    int status;

    fwk_list_push_tail(&fake_module_ctx.delayed_response_list,
                       &event[1].slist_node);
    fake_thread_module_ctx.waiting_event_processing_completion = true;
    event[0].source_id = FWK_ID_MODULE(0x2);
    event[0].target_id = FWK_ID_MODULE(0x1);
    event[0].is_response = true;
    event[0].cookie = 2;
    event[1].cookie = 2;
    fake_module_response_event.cookie = 2;
    fake_module_ctx.thread_ctx->response_event = &fake_module_response_event;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(ctx->thread_ready_queue.head == &fake_thread_module_ctx.slist_node);
    assert(ctx->thread_ready_queue.tail == &fake_thread_module_ctx.slist_node);
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
    assert(osThreadFlagsSet_count_call == 0);
}

static void test_put_event_and_wait_invalid_context(void)
{
    int status;

    /* Target context invalid */
    ctx->running = true;
    fwk_module_is_valid_module_id_return_val = false;

    status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_invalid_event_id(void)
{
    int status;

    /* Target context invalid */
    ctx->running = true;
    fwk_module_is_valid_event_id_return_val = false;

    status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_incompatible_target_event(void)
{
    int status;

    event[0].target_id = FWK_ID_MODULE(0x3);

    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);

    event[0].target_id = FWK_ID_MODULE(0x2);
    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_invalid_state(void)
{
    int status;

    /* Context is not running */
    ctx->running = false;

    status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_E_STATE);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_invalid_event(void)
{
    int status;

    /* Response event invalid */
    ctx->running = true;

    status = fwk_thread_put_event_and_wait(NULL, &event[2]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_invalid_response_event(void)
{
    int status;

    /* Response event invalid */
    ctx->running = true;

    status = fwk_thread_put_event_and_wait(&event[0], NULL);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_put_event_failed(void)
{
    int status;

    /* Put event function failed */
    ctx->running = true;
    ctx->current_event = NULL;
    fwk_list_push_tail(&ctx->event_free_queue, &event[2].slist_node);
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_module_is_valid_entity_id_return_val = false;

    status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_E_PARAM);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_called_from_isr(void)
{
    int status;

    /* Call from ISR */
    ctx->running = true;

    status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_E_ACCESS);

    assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_wait_flags_failed(void)
{
    int status;

    ctx->running = true;
    ctx->current_thread_ctx = &fake_thread_element_ctx;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    /* Flag wait failed */
    fwk_list_push_tail(&fake_module_ctx.thread_ctx->event_queue,
        &event[2].slist_node);
    osThreadFlagsWait_return_val[0] = 0;
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);

    status = fwk_thread_put_event_and_wait(&event[0], &event[3]);
    assert(status == FWK_E_OS);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[2].slist_node);
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(osThreadFlagsSet_count_call == 0);
    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAll);

    assert(ctx->current_thread_ctx == &fake_thread_element_ctx);
    assert(fake_thread_element_ctx.response_event == NULL);
    assert(fake_thread_element_ctx.waiting_event_processing_completion ==
           false);
    assert(ctx->event_cookie_counter == 1);
}

static void test_put_event_and_wait_thread_ready_queue_not_empty(void)
{
    volatile int status = FWK_SUCCESS;

    ctx->running = true;
    ctx->event_cookie_counter = 11;
    ctx->current_thread_ctx = &fake_thread_element_ctx;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    /* thread_ready_queue is not empty */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    osThreadFlagsWait_break = 0;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_PROCESSED;

    if (setjmp(test_context) == FWK_SUCCESS)
        status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_SUCCESS);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(osThreadFlagsSet_count_call == 1);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAll);

    assert(ctx->current_thread_ctx == &fake_thread_element_ctx);
    assert(fake_thread_element_ctx.response_event == &event[2]);
    assert(fake_thread_element_ctx.waiting_event_processing_completion == true);

    assert(fwk_id_is_equal(event[1].source_id, event[0].source_id));
    assert(fwk_id_is_equal(event[1].target_id, event[0].target_id));
    assert(event[1].cookie == 11);
    assert(event[1].is_response == false);
    assert(event[1].response_requested == true);
    assert(fwk_id_is_equal(event[1].id, event[0].id));
    assert(!memcmp(event[1].params, event[0].params, sizeof(event[1].params)));

    assert(fake_thread_element_ctx.response_event->cookie == 11);

    assert(ctx->event_cookie_counter == 12);
}

static void test_put_event_and_wait_thread_ready_queue_empty(void)
{
    int status;

    ctx->running = true;
    ctx->current_thread_ctx = &fake_thread_element_ctx;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    /* thread_ready_queue is empty */
    fwk_list_push_tail(&fake_module_ctx.thread_ctx->event_queue,
        &event[3].slist_node);
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_PROCESSED;

    status = fwk_thread_put_event_and_wait(&event[0], &event[2]);
    assert(status == FWK_SUCCESS);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[1].slist_node);
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[3].slist_node);
    assert(osThreadFlagsSet_count_call == 0);
    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAll);
    assert(ctx->event_cookie_counter == 1);
}

static void test_put_event_and_wait_called_from_common_thread(void)
{
    ctx->running = true;
    process_event_call_thread_put_event_and_wait = true;
    osThreadFlagsWait_break = 0;

    osThreadFlagsSet_return_val = 0;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;

    /*
     * thread_put_event_and_wait fails if it is called from the common
     * thread.
     */
    fwk_list_push_tail(&ctx->event_isr_queue, &event[2].slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[0].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);

    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(fwk_thread_put_event_and_wait_return_val == FWK_E_ACCESS);
    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAny);
    assert(osThreadFlagsSet_count_call == 1);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(ctx->event_cookie_counter == 1);
}

static void test_put_event_and_wait_called_from_current_thread(void)
{
    ctx->running = true;
    process_event_call_thread_put_event_and_wait = true;
    osThreadFlagsWait_break = 1;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;

    osThreadFlagsSet_return_val = 0;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    ctx->current_thread_ctx = &fake_thread_module_ctx;

    /* thread_put_event_and_wait fails if called from the current thread */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fwk_list_push_tail(&fake_module_ctx.thread_ctx->event_queue,
        &event[0].slist_node);

    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function(fake_module_ctx.thread_ctx);

    assert(fwk_thread_put_event_and_wait_return_val == FWK_E_ACCESS);
    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
        assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAny);
        assert(osThreadFlagsSet_count_call == 2);
        assert(
            osThreadFlagsSet_param_thread_id[0] ==
            (osThreadId_t)COMMON_THREAD_ID);
        assert(osThreadFlagsSet_param_flags[0] == SIGNAL_NO_READY_THREAD);
        assert(ctx->event_cookie_counter == 0);
}

static void test_put_event_and_wait_event_with_response(void)
{
    ctx->running = true;
    process_event_call_thread_put_event_and_wait = true;
    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;
    osThreadFlagsWait_return_val[1] = SIGNAL_EVENT_PROCESSED;

    osThreadFlagsSet_return_val = 0;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    ctx->current_thread_ctx = &fake_thread_element_ctx;

    /* event asking for a response */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fwk_list_push_tail(&ctx->event_free_queue, &event[3].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &fake_element_ctx.thread_ctx->slist_node);

    fwk_list_push_tail(&fake_element_ctx.thread_ctx->event_queue,
        &event[2].slist_node);
    fwk_list_push_tail(&fake_element_ctx.thread_ctx->event_queue,
        &event[0].slist_node);

    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function(fake_element_ctx.thread_ctx);

    assert(fwk_thread_put_event_and_wait_return_val == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
    assert(fwk_id_is_equal(event[1].source_id, FWK_ID_MODULE(0x6)));
    assert(fwk_id_is_equal(event[1].target_id, FWK_ID_MODULE(0x6)));
    assert(event[1].response_requested == true);
    assert(fake_element_ctx.thread_ctx->event_queue.head ==
        &event[0].slist_node);
    assert(fwk_id_is_equal(event[0].source_id, FWK_ID_MODULE(0x1)));
    assert(fwk_id_is_equal(event[0].target_id, FWK_ID_MODULE(0x2)));
    assert(ctx->event_free_queue.head == &event[2].slist_node);
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[3].slist_node);
    assert(fwk_id_is_equal(event[3].source_id, FWK_ID_MODULE(0x6)));
    assert(fwk_id_is_equal(event[3].target_id, FWK_ID_MODULE(0x5)));
    assert(event[3].is_response == true);
    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAny);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsWait_param_options[1] == osFlagsWaitAll);
    assert(osThreadFlagsSet_count_call == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);
    assert(ctx->event_cookie_counter == 2);
}

static void test_put_event_and_wait_event_without_response(void)
{
    ctx->running = true;
    process_event_call_thread_put_event_and_wait = true;
    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;
    osThreadFlagsWait_return_val[1] = SIGNAL_EVENT_PROCESSED;

    osThreadFlagsSet_return_val = 0;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    ctx->current_thread_ctx = &fake_thread_element_ctx;

    /* event not asking for a response */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);

    fwk_list_push_tail(&fake_element_ctx.thread_ctx->event_queue,
        &event[0].slist_node);

    fwk_list_push_tail(&ctx->thread_ready_queue,
        &fake_element_ctx.thread_ctx->slist_node);

    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function(fake_element_ctx.thread_ctx);

    assert(fwk_thread_put_event_and_wait_return_val == FWK_SUCCESS);

    assert(ctx->event_free_queue.head == &event[0].slist_node);

    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);

    assert(fwk_id_is_equal(event[1].source_id, FWK_ID_MODULE(0x2)));
    assert(fwk_id_is_equal(event[1].target_id, FWK_ID_MODULE(0x6)));
    assert(event[1].response_requested == true);

    assert(fwk_list_is_empty(&fake_element_ctx.thread_ctx->event_queue));

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_options[0] == osFlagsWaitAny);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsWait_param_options[1] == osFlagsWaitAll);
    assert(osThreadFlagsSet_count_call == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);
    assert(ctx->event_cookie_counter == 1);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_put_event_ctx_not_initialized),
    FWK_TEST_CASE(test_put_event_invalid_event),
    FWK_TEST_CASE(test_put_event_invalid_thread_context),
    FWK_TEST_CASE(test_put_event_incompatible_target_event),
    FWK_TEST_CASE(test_put_event_incompatible_target_notification),
    FWK_TEST_CASE(test_put_event_call_from_a_thread),
    FWK_TEST_CASE(test_put_event_invalid_entity_id),
    FWK_TEST_CASE(test_put_event_invalid_event_id),
    FWK_TEST_CASE(test_put_event_invalid_notification_id),
    FWK_TEST_CASE(test_put_event_notification_invalid_response_flag),
    FWK_TEST_CASE(test_put_event_notification_invalid_response_requested_flag),
    FWK_TEST_CASE(test_put_event_function_failed),
    FWK_TEST_CASE(test_put_event_one_isr_event),
    FWK_TEST_CASE(test_put_event_set_flags_failed),
    FWK_TEST_CASE(test_put_event_several_isr_events),
    FWK_TEST_CASE(test_put_event_to_waiting_thread_not_delayed_response),
    FWK_TEST_CASE(test_put_event_to_waiting_thread_not_wakeup_event),
    FWK_TEST_CASE(test_put_event_to_waiting_thread_wakeup_event),
    FWK_TEST_CASE(test_put_event_and_wait_invalid_context),
    FWK_TEST_CASE(test_put_event_and_wait_invalid_event_id),
    FWK_TEST_CASE(test_put_event_and_wait_incompatible_target_event),
    FWK_TEST_CASE(test_put_event_and_wait_invalid_state),
    FWK_TEST_CASE(test_put_event_and_wait_put_event_failed),
    FWK_TEST_CASE(test_put_event_and_wait_invalid_event),
    FWK_TEST_CASE(test_put_event_and_wait_invalid_response_event),
    FWK_TEST_CASE(test_put_event_and_wait_called_from_isr),
    FWK_TEST_CASE(test_put_event_and_wait_wait_flags_failed),
    FWK_TEST_CASE(test_put_event_and_wait_thread_ready_queue_not_empty),
    FWK_TEST_CASE(test_put_event_and_wait_thread_ready_queue_empty),
    FWK_TEST_CASE(test_put_event_and_wait_called_from_common_thread),
    FWK_TEST_CASE(test_put_event_and_wait_called_from_current_thread),
    FWK_TEST_CASE(test_put_event_and_wait_event_with_response),
    FWK_TEST_CASE(test_put_event_and_wait_event_without_response)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_multi_thread_put_event",
    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
