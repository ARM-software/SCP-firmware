/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_id.h>
#include <internal/fwk_module.h>
#include <internal/fwk_multi_thread.h>
#include <internal/fwk_notification.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SIGNAL_ISR_EVENT 0x01
#define SIGNAL_EVENT_TO_PROCESS 0x02
#define SIGNAL_EVENT_PROCESSED 0x04
#define SIGNAL_NO_READY_THREAD 0x08

#define MODULE_THREAD_ID    10
#define COMMON_THREAD_ID    11
#define SPECIFIC_THREAD_ID  12

static jmp_buf test_context;
static struct fwk_slist_node slist_node[6];
static struct fwk_event event[4];
static struct fwk_event notification[2];
static struct __fwk_multi_thread_ctx *ctx;

/* Mock module */
static struct __fwk_thread_ctx fake_thread_module_ctx;
static struct fwk_module fake_module;
static struct fwk_module_ctx fake_module_ctx;

static unsigned int process_event_call_count;
static struct fwk_event *process_event_event_to_put;
static const struct fwk_event *process_event_param_event[3];
static struct fwk_event *process_event_param_response_event[3];
static bool process_event_delay_response;
static int process_event(const struct fwk_event *event,
                         struct fwk_event *response_event)
{
    assert(process_event_call_count <
           FWK_ARRAY_SIZE(process_event_param_event));
    assert(process_event_call_count <
           FWK_ARRAY_SIZE(process_event_param_response_event));

    process_event_param_event[process_event_call_count] = event;
    process_event_param_response_event[process_event_call_count++] =
        response_event;

    if (process_event_event_to_put != NULL) {
        fwk_thread_put_event(process_event_event_to_put);
        process_event_event_to_put = NULL;
    }

    if (process_event_delay_response)
        response_event->is_delayed_response = true;

    return FWK_SUCCESS;
}

static unsigned int process_notification_call_count;
static const struct fwk_event *process_notification_param_event[3];
static int process_notification(const struct fwk_event *event,
                                struct fwk_event *response_event)
{
    (void)response_event;
    assert(process_notification_call_count <
           FWK_ARRAY_SIZE(process_notification_param_event));

    process_notification_param_event[process_notification_call_count++] = event;

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
static unsigned int osThreadFlagsWait_call_count;
static uint32_t osThreadFlagsWait_return_val[3];
static uint32_t osThreadFlagsWait_param_flags[3];
static uint32_t osThreadFlagsWait_param_options[3];
uint32_t __wrap_osThreadFlagsWait(uint32_t flags, uint32_t options,
                                  uint32_t timeout)
{
    (void) timeout;
    assert(osThreadFlagsWait_call_count <
           FWK_ARRAY_SIZE(osThreadFlagsWait_param_flags));
    assert(osThreadFlagsWait_call_count <
           FWK_ARRAY_SIZE(osThreadFlagsWait_param_options));

    osThreadFlagsWait_param_flags[osThreadFlagsWait_call_count] = flags;
    osThreadFlagsWait_param_options[osThreadFlagsWait_call_count] = options;

    if ((osThreadFlagsWait_call_count + 1) == osThreadFlagsWait_break) {
        osThreadFlagsWait_call_count = 0;
        longjmp(test_context, !FWK_SUCCESS);
    } else
        return osThreadFlagsWait_return_val[osThreadFlagsWait_call_count++];
}

static osThreadId_t osThreadFlagsSet_param_thread_id[3];
static uint32_t osThreadFlagsSet_param_flags[3];
static uint32_t osThreadFlagsSet_return_val[3];
static unsigned int osThreadFlagsSet_call_count;
uint32_t __wrap_osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    assert(osThreadFlagsSet_call_count <
           FWK_ARRAY_SIZE(osThreadFlagsSet_param_thread_id));
    assert(osThreadFlagsSet_call_count <
           FWK_ARRAY_SIZE(osThreadFlagsSet_param_flags));

    osThreadFlagsSet_param_thread_id[osThreadFlagsSet_call_count] = thread_id;
    osThreadFlagsSet_param_flags[osThreadFlagsSet_call_count] = flags;
    return osThreadFlagsSet_return_val[osThreadFlagsSet_call_count++];
}

uint32_t __wrap_osThreadFlagsClear(uint32_t flags)
{
    return osThreadFlagsSet_param_flags[osThreadFlagsSet_call_count];
}

static osThreadFunc_t common_thread_function;
static osThreadFunc_t specific_thread_function;
osThreadId_t __wrap_osThreadNew(osThreadFunc_t func, void *argument,
                                const osThreadAttr_t *attr)
{
    static unsigned int call_count = 0;
    assert(call_count < 3);

    (void) argument;
    (void) attr;

    if ((call_count++) == 0) {
        common_thread_function = func;
        return (osThreadId_t)COMMON_THREAD_ID;
    }

    specific_thread_function = func;
    return (osThreadId_t)SPECIFIC_THREAD_ID;
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

bool __wrap_fwk_module_is_valid_module_id(fwk_id_t id)
{
    return true;
}

bool __wrap_fwk_module_is_valid_entity_id(fwk_id_t id)
{
    return false;
}

bool __wrap_fwk_module_is_valid_event_id(fwk_id_t id)
{
    return true;
}

bool __wrap_fwk_module_is_valid_notification_id(fwk_id_t id)
{
    return true;
}

struct fwk_element_ctx *__wrap___fwk_module_get_element_ctx(fwk_id_t id)
{
    (void) id;
    return NULL;
}

struct fwk_module_ctx *__wrap___fwk_module_get_ctx(fwk_id_t id)
{
    (void) id;
    return &fake_module_ctx;
}

int __wrap___fwk_module_get_state(fwk_id_t id, enum fwk_module_state *state)
{
    (void) id;
    (void) state;
    return FWK_SUCCESS;
}

static int test_suite_setup(void)
{
    int status;

    ctx = __fwk_multi_thread_get_ctx();
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    status = __fwk_thread_init(2);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_thread_create(FWK_ID_MODULE(0x0));
}

static void test_case_setup(void)
{
    event[0].slist_node = slist_node[0];
    event[0].source_id = FWK_ID_MODULE(0x1);
    event[0].target_id = FWK_ID_MODULE(0x2);
    event[0].cookie = 0;
    event[0].is_response = false;
    event[0].response_requested = false;
    event[0].is_thread_wakeup_event = false;
    event[0].id = FWK_ID_EVENT(2, 7);

    event[1].slist_node = slist_node[1];
    event[1].source_id = FWK_ID_MODULE(0x3);
    event[1].target_id = FWK_ID_MODULE(0x4);
    event[1].cookie = 0;
    event[1].is_response = false;
    event[1].response_requested = false;
    event[1].is_thread_wakeup_event = false;
    event[1].id = FWK_ID_EVENT(4, 8);

    event[2].slist_node = slist_node[2];
    event[2].source_id = FWK_ID_MODULE(0x5);
    event[2].target_id = FWK_ID_MODULE(0x6);
    event[2].cookie = 0;
    event[2].is_response = false;
    event[2].response_requested = true;
    event[2].is_thread_wakeup_event = false;
    event[2].id = FWK_ID_EVENT(6, 9);
    memset(event[2].params, 0xA5, sizeof(event[2].params));

    event[3].slist_node = slist_node[3];
    event[3].source_id = FWK_ID_MODULE(0x7);
    event[3].target_id = FWK_ID_MODULE(0x8);
    event[3].cookie = 0;
    event[3].is_response = false;
    event[3].response_requested = false;
    event[3].is_thread_wakeup_event = false;
    event[3].id = FWK_ID_EVENT(8, 10);

    notification[0].slist_node = slist_node[4];
    notification[0].source_id = FWK_ID_MODULE(0x9);
    notification[0].target_id = FWK_ID_MODULE(0xA);
    notification[0].is_response = false;
    notification[0].response_requested = true;
    notification[0].is_notification = true;
    notification[0].id = FWK_ID_NOTIFICATION(9, 12);

    notification[1].slist_node = slist_node[5];
    notification[1].source_id = FWK_ID_MODULE(0xB);
    notification[1].target_id = FWK_ID_MODULE(0xC);
    notification[1].is_response = false;
    notification[1].response_requested = false;
    notification[1].is_notification = true;
    notification[1].id = FWK_ID_NOTIFICATION(11, 14);

    fwk_interrupt_get_current_return_val = FWK_SUCCESS;

    process_event_call_count = 0;
    process_event_event_to_put = NULL;
    process_notification_call_count = 0;
    process_event_delay_response = false;

    osThreadFlagsWait_break = 0;
    osThreadFlagsWait_call_count = 0;
    osThreadFlagsWait_param_flags[0] = 0;
    osThreadFlagsWait_param_flags[1] = 0;
    osThreadFlagsWait_param_flags[2] = 0;

    osThreadFlagsSet_call_count = 0;

    fake_thread_module_ctx.os_thread_id = (osThreadId_t)MODULE_THREAD_ID;
    fake_module.process_event = process_event;
    fake_module.process_notification = process_notification;
    fake_module_ctx.desc = &fake_module;
    fake_module_ctx.thread_ctx = &fake_thread_module_ctx;
    fake_thread_module_ctx.response_event = NULL;
    fake_thread_module_ctx.waiting_event_processing_completion = false;

    ctx->waiting_for_isr_event = false;
    ctx->event_cookie_counter = 0;
    fwk_list_init(&ctx->event_free_queue);
    fwk_list_init(&ctx->thread_ready_queue);
    fwk_list_init(&ctx->event_isr_queue);
    fwk_list_init(&ctx->common_thread_ctx.event_queue);
    fwk_list_init(&fake_thread_module_ctx.event_queue);
    fwk_list_init(&fake_module_ctx.delayed_response_list);
}

static void test_get_next_isr_event_1(void)
{
    /*
     * Test of get_next_isr_event() when the queue of ISR events is empty and
     * waiting for the SIGNAL_ISR_EVENT signal.
     * No event of any kind to process, the execution proceeds to wait for an
     * ISR event and is stopped.
     */
    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_get_next_isr_event_2(void)
{
    /*
     * Test of get_next_isr_event() when the queue of ISR events is empty and
     * waiting for the SIGNAL_ISR_EVENT signal fails.
     * No event of any kind to process, the execution proceeds to waiting
     * for an ISR event. The osThreadFlagsWait() function returns with the
     * invalid value 0, the execution loops to waiting for an ISR event
     * again where the execution ends.
     */
    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[2] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_get_next_isr_event_3(void)
{
    /*
     * Test of get_next_isr_event() when:
     * 1) there is an event in the ISR queue
     * 2) the thread target of the event is not waiting for the processing
     *    completion of an event.
     *
     * The ISR event is extracted and put into the specific thread event
     * queue. The thread is put into the queue of the ready to process
     * an event thread. The execution proceeds to wait for the
     * SIGNAL_EVENT_TO_PROCESS or SIGNAL_NO_READY_THREAD signal.
     */

    fwk_list_push_tail(&ctx->event_isr_queue, &event[0].slist_node);
    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[0].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[0].slist_node);

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 1);
}

static void test_get_next_isr_event_4(void)
{
    /*
     * Test of get_next_isr_event() when:
     * 1) there is an event in the ISR queue
     * 2) the thread target of the event is waiting for the processing
     *    completion of an event.
     * 3) the ISR event is not a response event thus not the completion event
     *    the thread is waiting for.
     * The ISR event is extracted and put into the specific thread event
     * queue but the thread is not put into the queue of the ready to process
     * an event thread. The execution proceeds to wait for an ISR event where
     * the execution is stopped.
     */
    struct fwk_event response_event;

    ctx->event_cookie_counter = 7;
    fwk_list_push_tail(&ctx->event_isr_queue, &event[0].slist_node);
    response_event.cookie = 4;
    fake_thread_module_ctx.response_event = &response_event;
    fake_thread_module_ctx.waiting_event_processing_completion = true;

    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[0].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[0].slist_node);

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 8);
}

static void test_get_next_isr_event_5(void)
{
    /*
     * Test of get_next_isr_event() when:
     * 1) there is an event in the ISR queue
     * 2) the thread target of the event is waiting for the processing
     *    completion of an event.
     * 3) the ISR event is a response event but not the completion event
     *    the thread is waiting for.
     * The ISR event is extracted and put into the specific thread event
     * queue but the thread is not put into the queue of the ready to process
     * an event thread. The execution proceeds to wait for an ISR event where
     * the execution is stopped.
     */
    struct fwk_event response_event;

    ctx->event_cookie_counter = 7;
    event[0].cookie = 5;
    event[0].is_response = true;
    fwk_list_push_tail(&ctx->event_isr_queue, &event[0].slist_node);
    response_event.cookie = 6;
    fake_thread_module_ctx.response_event = &response_event;
    fake_thread_module_ctx.waiting_event_processing_completion = true;

    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[0].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[0].slist_node);

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 8);
}

static void test_get_next_isr_event_6(void)
{
    /*
     * Test of get_next_isr_event() when:
     * 1) there is an event in the ISR queue
     * 2) the thread target of the event is waiting for the processing
     *    completion of an event.
     * 3) the ISR event is the completion event the thread is waiting for.
     *
     * The ISR event is extracted and put at the head of the specific thread
     * event queue. The thread is put at the head of the processes ready to
     * process an event. When looking for the next event to process, the thread
     * is removed for the thread ready queue, the event removed from the
     * thread's event queue, copied into the thread context as the response
     * event the thread is waiting for and released.
     */
    struct fwk_event response_event;

    ctx->event_cookie_counter = 7;
    event[0].cookie = 5;
    event[0].is_response = true;
    fwk_list_push_tail(&ctx->event_isr_queue, &event[0].slist_node);
    response_event.cookie = 5;
    fake_thread_module_ctx.response_event = &response_event;
    fake_thread_module_ctx.waiting_event_processing_completion = true;

    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(fwk_id_is_equal(response_event.source_id, event[0].source_id));
    assert(fwk_id_is_equal(response_event.target_id, event[0].target_id));
    assert(fwk_id_is_equal(response_event.id, event[0].id));
    assert(response_event.cookie == 7);
    assert(response_event.is_response == true);
    assert(response_event.response_requested == false);
    assert(response_event.is_thread_wakeup_event == true);
    assert(response_event.is_delayed_response == false);
    assert(!memcmp(response_event.params, event[0].params,
                   sizeof(response_event.params)));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 8);
}

static void test_launch_next_event_processing_1(void)
{
    /*
     * Test of launch_next_event_processing() with a non-empty queue of ready
     * threads with the thread at the head of this list being the current one.
     *
     * The common thread is removed from the queue of ready threads and the
     * event is processed. Then, the execution within the common thread proceeds
     * to wait the SIGNAL_ISR_EVENT signal where the execution is stopped.
     */
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[3].slist_node);

    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[3]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(ctx->event_free_queue.head == &event[3].slist_node);
    assert(ctx->event_free_queue.tail == &event[3].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_launch_next_event_processing_2(void)
{
    /*
     * Test of launch_next_event_processing() with:
     * 1) a non-empty queue of ready threads
     * 2) the thread at the head of this list is not the current one
     * 3) the signalling of the event to process to the target thread succeeds
     *
     * The specific thread is removed from the queue of ready threads and the
     * SIGNAL_EVENT_TO_PROCESS signal is set for it. Then, the execution
     * within the common thread proceeds to wait the SIGNAL_EVENT_TO_PROCESS or
     * SIGNAL_NO_READY_THREAD where the execution is stopped.
     */
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &fake_thread_module_ctx.slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[3].slist_node);

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[3].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[3].slist_node);

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 0);
}

static void test_launch_next_event_processing_3(void)
{
    /*
     * Test of launch_next_event_processing() with:
     * 1) a non-empty queue of ready threads
     * 2) the thread at the head of the ready thread queue has one event in its
     *    event queue.
     * 3) the thread at the head of the ready thread queue is not the current
     *    one.
     * 4) the signalling of the event to process to the target thread fails.
     *
     * The specific thread is removed from the queue of ready threads and the
     * SIGNAL_EVENT_TO_PROCESS signal is set for it but it fails. Then, the
     * execution within the common thread proceeds to wait the SIGNAL_ISR_EVENT
     * signal where the execution is stopped.
     */
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &fake_thread_module_ctx.slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[0].slist_node);

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = -1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_launch_next_event_processing_4(void)
{
    /*
     * Test of launch_next_event_processing() with:
     * 1) a non-empty queue of ready threads
     * 2) the thread at the head of the ready thread queue has two events in its
     *    event queue.
     * 3) the thread at the head of the ready thread queue is not the current
     *    one.
     * 4) the signalling of the events to process to the target thread fails.
     *
     * The specific thread is removed from the queue of ready threads and the
     * SIGNAL_EVENT_TO_PROCESS signal is set for it but it fails. As there is
     * another event in the specific thread queue, the specific thread is put
     * back into the queue of the ready thread. Then the specific thread is
     * removed again from the queue of ready threads, the signalling of
     * the event fails again and the execution within the common thread
     * proceeds to wait the SIGNAL_ISR_EVENT signal where the execution is
     * stopped.
     */
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &fake_thread_module_ctx.slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[0].slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[1].slist_node);

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = -1;
    osThreadFlagsSet_return_val[1] = -1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 2);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);
    assert(osThreadFlagsSet_param_flags[1] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[1] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_1(void)
{
    /*
     * Test of thread_function(), waiting for an event within a specific thread.
     */
    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function((void *)&fake_thread_module_ctx);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_2(void)
{
    /*
     * Test of thread_function(), waiting for an event within a specific thread,
     * the osThreadFlagsWait() function returns an unexpected value.
     */
    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function((void *)&fake_thread_module_ctx);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[2] == 0);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(fwk_list_is_empty(&ctx->event_free_queue));
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_3(void)
{
    /*
     * Test of thread_function(). The event added to the common thread event
     * queue is processed within the common thread which then proceeds to wait
     * for an ISR event.
     */
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[3].slist_node);

    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[3]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(ctx->event_free_queue.head == &event[3].slist_node);
    assert(ctx->event_free_queue.tail == &event[3].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_4(void)
{
    /*
     * Test of thread_function().
     * 1) The event  added to the specific thread event queue is processed
     *    within the common thread.
     * 2) As part of the processing of the event an event is sent to the
     *    common thread.
     * 3) The second event is processed within the common thread which then
     *    proceeds to wait for an ISR event.
     */
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[0].slist_node);
    process_event_event_to_put = &event[1];
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_list_push_tail(&ctx->event_free_queue, &event[2].slist_node);

    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;
    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function((void *)&fake_thread_module_ctx);

    assert(process_event_call_count == 2);
    assert(process_event_param_event[0] == &event[0]);
    assert(process_event_param_event[1] == &event[2]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[2] == 0);

    assert(osThreadFlagsSet_call_count == 2);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_NO_READY_THREAD);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)COMMON_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[2].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 1);
}

static void test_thread_function_5(void)
{
    /*
     * Test of thread_function(). The event, for which a response is requested,
     * is processed within the common thread. The specific thread is waiting for
     * the event completion, thus the common thread signals the event has been
     * processed but the signalling fails. The execution then proceeds to wait
     * for an ISR event.
     */
    fwk_list_push_tail(&ctx->event_free_queue, &event[0].slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[2].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fake_thread_module_ctx.response_event = &event[1];
    fake_thread_module_ctx.waiting_event_processing_completion = true;

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = -1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[2]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[2].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 1);
}

static void test_thread_function_6(void)
{
    /*
     * Test of thread_function(). The event, for which a response is requested,
     * is processed within the common thread. The specific thread is waiting for
     * the event completion, thus the common thread signals the event has been
     * processed. The signalling succeeds and the execution proceeds to wait for
     * the SIGNAL_EVENT_TO_PROCESS or SIGNAL_NO_READY_THREAD signal.
     */
    struct fwk_event response_event = { };

    fwk_list_push_tail(&ctx->event_free_queue, &event[0].slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[2].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fake_thread_module_ctx.response_event = &response_event;
    fake_thread_module_ctx.waiting_event_processing_completion = true;

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[2]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[2].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(fwk_id_is_equal(response_event.source_id, event[2].target_id));
    assert(fwk_id_is_equal(response_event.target_id, event[2].source_id));
    assert(fwk_id_is_equal(response_event.id, event[2].id));
    assert(response_event.cookie == 0);
    assert(response_event.is_response == true);
    assert(response_event.response_requested == false);
    assert(response_event.is_thread_wakeup_event == true);
    assert(!memcmp(response_event.params, event[2].params,
           sizeof(response_event.params)));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 1);
}

static void test_thread_function_7(void)
{
    /*
     * Test of thread_function(). The event, for which a response is requested,
     * is processed within the common thread. The specific thread is waiting for
     * the event completion but the processing function set the
     * 'is_delayed_response' flag of the response event, thus the common thread
     * does not signals the event has been processed. The execution then
     * proceeds to wait for an ISR event.
     */
    ctx->event_cookie_counter = 5;
    event[2].cookie = 4;
    fwk_list_push_tail(&ctx->event_free_queue, &event[0].slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[2].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fake_thread_module_ctx.response_event = &event[1];
    fake_thread_module_ctx.waiting_event_processing_completion = true;
    process_event_delay_response = true;

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[2]);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(ctx->event_free_queue.head == &event[2].slist_node);
    assert(ctx->event_free_queue.tail == &event[2].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(fake_module_ctx.delayed_response_list.head == &event[0].slist_node);
    assert(fake_module_ctx.delayed_response_list.tail == &event[0].slist_node);

    assert(fwk_id_is_equal(event[0].source_id, event[2].target_id));
    assert(fwk_id_is_equal(event[0].target_id, event[2].source_id));
    assert(fwk_id_is_equal(event[0].id, event[2].id));
    assert(event[0].cookie == 4);
    assert(event[0].is_response == true);
    assert(event[0].response_requested == false);
    assert(event[0].is_thread_wakeup_event == false);
    assert(!memcmp(event[0].params, event[2].params, sizeof(event[0].params)));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 5);
}

static void test_thread_function_8(void)
{
    /*
     * Test of thread_function(), processing of two events in a row within a
     * specific thread, the two events being the only ones in the event queues
     * of the threads.
     * 1) The first event is processed.
     * 2) The second event is processed.
     * 3) There is no ready thread thus the thread signals it to the common
     *    thread.
     */
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[0].slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[1].slist_node);

    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;
    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function((void *)&fake_thread_module_ctx);

    assert(process_event_call_count == 2);
    assert(process_event_param_event[0] == &event[0]);
    assert(process_event_param_event[1] == &event[1]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[2] == 0);

    assert(osThreadFlagsSet_call_count == 2);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_NO_READY_THREAD);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)COMMON_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_9(void)
{
    /*
     * Test of thread_function(). Two events to process, the first one in the
     * common thread, the second one in the specific thread. The first event
     * is processed. As the second has to be processed in the specific thread,
     * the common thread signals it to the specific thread and proceeds to wait
     * for the SIGNAL_EVENT_TO_PROCESS or SIGNAL_NO_READY_THREAD signals.
     */

    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[0].slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[1].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &fake_thread_module_ctx.slist_node);

    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[0]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[1].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[1].slist_node);

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_10(void)
{
    /*
     * Test of thread_function(), processing of an event within a specific
     * thread, the event being the only one in the event queues of the threads.
     * 1) The event is processed.
     * 2) There is no ready thread thus the thread signals it to the common
     *    thread.
     */
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[0].slist_node);

    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;
    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function((void *)&fake_thread_module_ctx);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[0]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[2] == 0);

    assert(osThreadFlagsSet_call_count == 2);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_NO_READY_THREAD);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)COMMON_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 0);
}

static void test_thread_function_11(void)
{
    /*
     * Test of thread_function(), processing of an event within the common
     * thread, the event being the only one in the event queues of the threads.
     * 1) The event is processed.
     * 2) There is no other event to process thus the function thread_function()
     *    returns and the execution proceeds to wait for an ISR event.
     */
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[0].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);

    osThreadFlagsWait_break = 1;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[0]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
    assert(ctx->event_cookie_counter == 0);
}

static void test_process_next_thread_event_1(void)
{
    /*
     * Test of process_next_thread_event() with an event requesting a response.
     * No other thread is waiting for the response event.
     *
     * 1) The event is processed within the common thread.
     * 2) Its response is put in the specific thread event queue.
     * 3) The common thread signals the response event to the specific thread.
     * 4) The execution proceeds to wait for the SIGNAL_EVENT_TO_PROCESS or
     *    SIGNAL_NO_READY_THREAD where it is stopped.
     */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[2].slist_node);
    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[2]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[2].slist_node);
    assert(ctx->event_free_queue.tail == &event[2].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[1].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[1].slist_node);

    assert(fwk_id_is_equal(event[1].source_id, event[2].target_id));
    assert(fwk_id_is_equal(event[1].target_id, event[2].source_id));
    assert(fwk_id_is_equal(event[1].id, event[2].id));
    assert(event[1].cookie == 0);
    assert(event[1].is_response == true);
    assert(event[1].response_requested == false);
    assert(event[1].is_thread_wakeup_event == false);
    assert(!memcmp(event[1].params, event[2].params, sizeof(event[1].params)));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 1);
}

static void test_process_next_thread_event_2(void)
{
    /*
     * Test of process_next_thread_event() with an event requesting a response.
     * The specific thread is waiting for the response event.
     *
     * 1) The event is processed within the common thread.
     * 2) The response event pointed to by the specific thread context is
     *    filled in.
     * 3) The common thread raises the SIGNAL_EVENT_PROCESSED signal.
     * 4) The execution proceeds to wait for the SIGNAL_EVENT_TO_PROCESS or
     *    SIGNAL_NO_READY_THREAD where it is stopped.
     */
    struct fwk_event response_event = { };

    fwk_list_push_tail(&ctx->event_free_queue, &event[0].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &event[2].slist_node);
    fake_thread_module_ctx.response_event = &response_event;
    fake_thread_module_ctx.waiting_event_processing_completion = true;
    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 1);
    assert(process_event_param_event[0] == &event[2]);
    assert(process_notification_call_count == 0);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_PROCESSED);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[2].slist_node);
    assert(ctx->event_free_queue.tail == &event[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(fwk_id_is_equal(response_event.source_id, event[2].target_id));
    assert(fwk_id_is_equal(response_event.target_id, event[2].source_id));
    assert(fwk_id_is_equal(response_event.id, event[2].id));
    assert(response_event.cookie == 0);
    assert(response_event.is_response == true);
    assert(response_event.response_requested == false);
    assert(response_event.is_thread_wakeup_event == true);
    assert(!memcmp(response_event.params, event[2].params,
           sizeof(response_event.params)));

    assert(ctx->waiting_for_isr_event == false);
    assert(ctx->event_cookie_counter == 1);
}

static void test_process_next_thread_event_3(void)
{
    /*
     * Test of process_next_thread_event(), processing of two events in a row
     * within a specific thread, the two events being the only ones in the event
     * queues of the threads.
     * 1) The first event is processed within the specific thread.
     * 2) As the specific thread event queue is not empty, the specific thread
     *    is put back into the list of ready threads.
     * 2) The second event is processed within the specific thread.
     * 3) There is no ready thread thus the thread signals it to the common
     *    thread.
     */
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[0].slist_node);
    fwk_list_push_tail(&fake_thread_module_ctx.event_queue,
        &event[1].slist_node);

    osThreadFlagsWait_break = 2;
    osThreadFlagsWait_return_val[0] = SIGNAL_EVENT_TO_PROCESS;
    if (setjmp(test_context) == FWK_SUCCESS)
        specific_thread_function((void *)&fake_thread_module_ctx);

    assert(process_event_call_count == 2);
    assert(process_event_param_event[0] == &event[0]);
    assert(process_event_param_event[1] == &event[1]);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[1] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsWait_param_flags[2] == 0);

    assert(osThreadFlagsSet_call_count == 2);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_NO_READY_THREAD);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)COMMON_THREAD_ID);

    assert(ctx->event_free_queue.head == &event[0].slist_node);
    assert(ctx->event_free_queue.tail == &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == false);
}

static void test_process_next_thread_event_4(void)
{
    /*
     * Test of process_next_thread_event() with a notification requesting a
     * response.
     *
     * 1) The notification event is processed within the common thread.
     * 2) Its response is put in the specific thread event queue.
     * 3) The common thread signals the response event to the specific thread.
     * 4) The execution proceeds to wait for the SIGNAL_EVENT_TO_PROCESS or
     *    SIGNAL_NO_READY_THREAD where it is stopped.
     */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &notification[0].slist_node);
    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 1);
    assert(process_notification_param_event[0] == &notification[0]);

    assert(osThreadFlagsWait_param_flags[0] == (SIGNAL_EVENT_TO_PROCESS |
                                                SIGNAL_NO_READY_THREAD));
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 1);
    assert(osThreadFlagsSet_param_flags[0] == SIGNAL_EVENT_TO_PROCESS);
    assert(osThreadFlagsSet_param_thread_id[0] ==
        (osThreadId_t)MODULE_THREAD_ID);

    assert(ctx->event_free_queue.head == &notification[0].slist_node);
    assert(ctx->event_free_queue.tail == &notification[0].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fake_thread_module_ctx.event_queue.head == &event[1].slist_node);
    assert(fake_thread_module_ctx.event_queue.tail == &event[1].slist_node);

    assert(fwk_id_is_equal(event[1].source_id, notification[0].target_id));
    assert(fwk_id_is_equal(event[1].target_id, notification[0].source_id));
    assert(event[1].is_response == true);
    assert(event[1].response_requested == false);
    assert(event[1].is_notification == true);
    assert(!memcmp(event[1].params, notification[0].params,
           sizeof(event[1].params)));

    assert(ctx->waiting_for_isr_event == false);
}

static void test_process_next_thread_event_5(void)
{
    /*
     * Test of process_next_thread_event() with a notification not requesting a
     * response.
     *
     * 1) The notification event is processed within the common thread.
     * 2) The execution proceeds to wait for an ISR event.
     */
    fwk_list_push_tail(&ctx->thread_ready_queue,
        &ctx->common_thread_ctx.slist_node);
    fwk_list_push_tail(&ctx->common_thread_ctx.event_queue,
        &notification[1].slist_node);
    osThreadFlagsWait_break = 1;
    osThreadFlagsSet_return_val[0] = 0;
    if (setjmp(test_context) == FWK_SUCCESS)
        common_thread_function(NULL);

    assert(process_event_call_count == 0);
    assert(process_notification_call_count == 1);
    assert(process_notification_param_event[0] == &notification[1]);

    assert(osThreadFlagsWait_param_flags[0] == SIGNAL_ISR_EVENT);
    assert(osThreadFlagsWait_param_flags[1] == 0);

    assert(osThreadFlagsSet_call_count == 0);

    assert(ctx->event_free_queue.head == &notification[1].slist_node);
    assert(ctx->event_free_queue.tail == &notification[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_isr_queue));
    assert(fwk_list_is_empty(&ctx->common_thread_ctx.event_queue));
    assert(fwk_list_is_empty(&fake_thread_module_ctx.event_queue));

    assert(ctx->waiting_for_isr_event == true);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_get_next_isr_event_1),
    FWK_TEST_CASE(test_get_next_isr_event_2),
    FWK_TEST_CASE(test_get_next_isr_event_3),
    FWK_TEST_CASE(test_get_next_isr_event_4),
    FWK_TEST_CASE(test_get_next_isr_event_5),
    FWK_TEST_CASE(test_get_next_isr_event_6),
    FWK_TEST_CASE(test_launch_next_event_processing_1),
    FWK_TEST_CASE(test_launch_next_event_processing_2),
    FWK_TEST_CASE(test_launch_next_event_processing_3),
    FWK_TEST_CASE(test_launch_next_event_processing_4),
    FWK_TEST_CASE(test_thread_function_1),
    FWK_TEST_CASE(test_thread_function_2),
    FWK_TEST_CASE(test_thread_function_3),
    FWK_TEST_CASE(test_thread_function_4),
    FWK_TEST_CASE(test_thread_function_5),
    FWK_TEST_CASE(test_thread_function_6),
    FWK_TEST_CASE(test_thread_function_7),
    FWK_TEST_CASE(test_thread_function_8),
    FWK_TEST_CASE(test_thread_function_9),
    FWK_TEST_CASE(test_thread_function_10),
    FWK_TEST_CASE(test_thread_function_11),
    FWK_TEST_CASE(test_process_next_thread_event_1),
    FWK_TEST_CASE(test_process_next_thread_event_2),
    FWK_TEST_CASE(test_process_next_thread_event_3),
    FWK_TEST_CASE(test_process_next_thread_event_4),
    FWK_TEST_CASE(test_process_next_thread_event_5),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_multi_thread_common_thread",
    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
