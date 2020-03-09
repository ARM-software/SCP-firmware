/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/fwk_module.h>
#include <internal/fwk_single_thread.h>
#include <internal/fwk_thread.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_list.h>
#include <fwk_macros.h>
#include <fwk_slist.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>

static jmp_buf test_context;
static struct __fwk_thread_ctx *ctx;
static struct fwk_element_ctx fake_element_ctx;

/* Mock functions */
static void * fwk_mm_calloc_val;
static int fwk_mm_calloc_return_val;
void *__wrap_fwk_mm_calloc(size_t num, size_t size)
{
    if (fwk_mm_calloc_return_val) {
        fwk_mm_calloc_val = (void *)calloc(num, size);
        return fwk_mm_calloc_val;
    }
    return NULL;
}

static struct fwk_module fake_module_desc;
static struct fwk_module_ctx fake_module_ctx;
struct fwk_module_ctx *__wrap___fwk_module_get_ctx(fwk_id_t id)
{
    return &fake_module_ctx;
}

bool free_event_queue_break;
extern void __real___fwk_slist_push_tail(struct fwk_slist *restrict list,
    struct fwk_slist_node *restrict node);
void __wrap___fwk_slist_push_tail(
    struct fwk_slist *restrict list,
    struct fwk_slist_node *restrict new)
{
    __real___fwk_slist_push_tail(list, new);
    if (free_event_queue_break &&
        (list == &(ctx->free_event_queue)))
        longjmp(test_context, FWK_SUCCESS);
}

static bool is_valid_entity_id_return_val;
bool __wrap_fwk_module_is_valid_entity_id(fwk_id_t id)
{
    return is_valid_entity_id_return_val;
}

static bool is_valid_event_id_return_val;
bool __wrap_fwk_module_is_valid_event_id(fwk_id_t id)
{
    return is_valid_event_id_return_val;
}

static bool is_valid_notification_id_return_val;
bool __wrap_fwk_module_is_valid_notification_id(fwk_id_t id)
{
    return is_valid_notification_id_return_val;
}

struct fwk_element_ctx *__wrap___fwk_module_get_element_ctx(fwk_id_t id)
{
    (void)id;
    return &fake_element_ctx;
}

int __wrap_fwk_interrupt_global_enable(void)
{
    return FWK_SUCCESS;
}

int __wrap_fwk_interrupt_global_disable(void)
{
    return FWK_SUCCESS;
}

static int interrupt_get_current_return_val;
int __wrap_fwk_interrupt_get_current(unsigned int *interrupt)
{
    return interrupt_get_current_return_val;
}

static const struct fwk_event *processed_event;
static int process_event(const struct fwk_event *event,
                         struct fwk_event *response_event)
{
    processed_event = event;
    return FWK_SUCCESS;
}

static const struct fwk_event *processed_notification;

static int process_notification(const struct fwk_event *event,
                                struct fwk_event *response_event)
{
    processed_notification = event;
    return FWK_SUCCESS;
}

static int test_suite_setup(void)
{
    ctx = __fwk_thread_get_ctx();
    fake_module_desc.process_event = process_event;
    fake_module_desc.process_notification = process_notification;
    fake_module_ctx.desc = &fake_module_desc;
    return FWK_SUCCESS;
}

static void test_case_setup(void)
{
    free_event_queue_break = false;
    is_valid_entity_id_return_val = true;
    is_valid_event_id_return_val = true;
    is_valid_notification_id_return_val = true;
    interrupt_get_current_return_val = FWK_E_STATE;
    fwk_mm_calloc_return_val = true;
    fake_module_desc.process_event = process_event;
    fake_module_ctx.desc = &fake_module_desc;
}

static void test_case_teardown(void)
{
    *ctx = (struct __fwk_thread_ctx){ };
    fwk_list_init(&ctx->free_event_queue);
    fwk_list_init(&ctx->event_queue);
    fwk_list_init(&ctx->isr_event_queue);
}

static void test___fwk_thread_init(void)
{
    int result;
    size_t event_count = 2;

    fwk_mm_calloc_return_val = true;

    /* Insert 2 events in the list */
    result = __fwk_thread_init(event_count);
    assert(result == FWK_SUCCESS);
    assert(ctx->free_event_queue.head ==
        &(((struct fwk_event *)fwk_mm_calloc_val)->slist_node));
    assert(ctx->free_event_queue.tail ==
        &((((struct fwk_event *)(fwk_mm_calloc_val))+1)->slist_node));
}

static void test___fwk_thread_run(void)
{
    int result;
    struct fwk_event *free_event, *allocated_event;

    struct fwk_event event1 = {
        .source_id = FWK_ID_MODULE(0x1),
        .target_id = FWK_ID_MODULE(0x2),
        .is_response = false,
        .response_requested = true,
        .is_notification = false,
        .id = FWK_ID_EVENT(0x2, 0x7),
    };

    struct fwk_event event2 = {
        .source_id = FWK_ID_MODULE(0x3),
        .target_id = FWK_ID_MODULE(0x4),
        .is_response = false,
        .response_requested = false,
        .is_notification = true,
        .id = FWK_ID_NOTIFICATION(0x4, 0x8),
    };

    struct fwk_event event3 = {
        .source_id = FWK_ID_MODULE(0x5),
        .target_id = FWK_ID_MODULE(0x6),
        .is_response = false,
        .response_requested = false,
        .is_notification = false,
        .id = FWK_ID_EVENT(0x6, 0x9),
    };

    struct fwk_event notification1 = {
        .source_id = FWK_ID_MODULE(0x5),
        .target_id = FWK_ID_MODULE(0x6),
        .is_response = false,
        .response_requested = true,
        .is_notification = true,
        .id = FWK_ID_NOTIFICATION(0x5, 0x9),
    };

    result = __fwk_thread_init(1);
    assert(result == FWK_SUCCESS);
    free_event_queue_break = true;
    allocated_event = FWK_LIST_GET(fwk_list_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);

    __real___fwk_slist_push_tail(&ctx->event_queue, &(event1.slist_node));
    __real___fwk_slist_push_tail(&ctx->event_queue, &(event2.slist_node));
    __real___fwk_slist_push_tail(&ctx->isr_event_queue, &(event3.slist_node));
    __real___fwk_slist_push_tail(&ctx->isr_event_queue,
                                 &(notification1.slist_node));

    /* Event1 processing */
    if (setjmp(test_context) == FWK_SUCCESS)
        __fwk_thread_run();
    assert(ctx->isr_event_queue.head == &(event3.slist_node));
    assert(ctx->isr_event_queue.tail == &(notification1.slist_node));
    assert(ctx->event_queue.head == &(event2.slist_node));
    assert(ctx->event_queue.tail == &(allocated_event->slist_node));

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);
    assert(fwk_list_is_empty(&ctx->free_event_queue));
    assert(free_event == &event1);
    assert(processed_event == &event1);
    assert(processed_event->is_response == false);
    assert(processed_event->response_requested == true);
    assert(processed_event->is_notification == false);

    /* Event2 processing */
    if (setjmp(test_context) == FWK_SUCCESS)
        __fwk_thread_run();
    assert(ctx->isr_event_queue.head == &(event3.slist_node));
    assert(ctx->isr_event_queue.tail == &(notification1.slist_node));
    assert(ctx->event_queue.head == &(allocated_event->slist_node));
    assert(ctx->event_queue.tail == &(allocated_event->slist_node));

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);
    assert(fwk_list_is_empty(&ctx->free_event_queue));
    assert(free_event == &event2);
    assert(processed_notification == &event2);
    assert(processed_notification->is_response == false);
    assert(processed_notification->response_requested == false);
    assert(processed_notification->is_notification == true);

    /* Response to Event1 processing */
    if (setjmp(test_context) == FWK_SUCCESS)
        __fwk_thread_run();
    assert(ctx->isr_event_queue.head == &(event3.slist_node));
    assert(ctx->isr_event_queue.tail == &(notification1.slist_node));
    assert(fwk_list_is_empty(&ctx->event_queue));

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);
    assert(free_event == allocated_event);
    assert(processed_event == allocated_event);
    assert(processed_event->is_response == true);
    assert(processed_event->response_requested == false);
    assert(processed_event->is_notification == false);
    assert(fwk_id_is_equal(processed_event->source_id, FWK_ID_MODULE(0x2)));
    assert(fwk_id_is_equal(processed_event->target_id, FWK_ID_MODULE(0x1)));
    assert(fwk_id_is_equal(processed_event->id, FWK_ID_EVENT(0x2, 0x7)));

    /* Extract ISR Event3 and process it */
    if (setjmp(test_context) == FWK_SUCCESS)
        __fwk_thread_run();
    assert(ctx->isr_event_queue.head == &(notification1.slist_node));
    assert(ctx->isr_event_queue.tail == &(notification1.slist_node));
    assert(fwk_list_is_empty(&ctx->event_queue));

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);
    assert(free_event == &event3);
    assert(processed_event == &event3);
    assert(processed_event->is_response == false);
    assert(processed_event->response_requested == false);
    assert(processed_event->is_notification == false);

    /* Extract ISR Notification1 and process it */
    free_event_queue_break = false;
    fwk_list_push_tail(&ctx->free_event_queue, &(allocated_event->slist_node));
    free_event_queue_break = true;
    if (setjmp(test_context) == FWK_SUCCESS)
        __fwk_thread_run();
    assert(fwk_list_is_empty(&ctx->isr_event_queue));
    assert(ctx->event_queue.head == &(allocated_event->slist_node));
    assert(ctx->event_queue.tail == &(allocated_event->slist_node));

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);
    assert(free_event == &notification1);
    assert(processed_notification == &notification1);
    assert(processed_notification->is_response == false);
    assert(processed_notification->response_requested == true);
    assert(processed_notification->is_notification == true);

    /* Process response to Notification1 */
    if (setjmp(test_context) == FWK_SUCCESS)
        __fwk_thread_run();
    assert(fwk_list_is_empty(&ctx->isr_event_queue));
    assert(fwk_list_is_empty(&ctx->event_queue));

    free_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->free_event_queue),
        struct fwk_event, slist_node);
    assert(free_event == allocated_event);
    assert(processed_notification == allocated_event);
    assert(processed_notification->is_response == true);
    assert(processed_notification->response_requested == false);
    assert(processed_notification->is_notification == true);
    assert(fwk_id_is_equal(processed_notification->source_id,
                           FWK_ID_MODULE(0x6)));
    assert(fwk_id_is_equal(processed_notification->target_id,
                           FWK_ID_MODULE(0x5)));
    assert(fwk_id_is_equal(processed_notification->id,
                           FWK_ID_NOTIFICATION(0x5, 0x9)));
}

static void test_fwk_thread_put_event(void)
{
    int result;
    struct fwk_event *result_event;

    struct fwk_event event1 = {
        .source_id = FWK_ID_MODULE(0x1),
        .target_id = FWK_ID_MODULE(0x2),
        .is_response = false,
        .response_requested = true,
        .id = FWK_ID_EVENT(0x2, 7),
    };

    struct fwk_event event2 = {
        .source_id = FWK_ID_MODULE(0x3),
        .target_id = FWK_ID_MODULE(0x4),
        .is_response = true,
        .response_requested = false,
    };

    /* Thread not initialized */
    result = fwk_thread_put_event(&event2);
    assert(result == FWK_E_INIT);

    result = __fwk_thread_init(2);
    assert(result == FWK_SUCCESS);

    /* Invalid entity ID */
    is_valid_entity_id_return_val = false;
    result = fwk_thread_put_event(&event2);
    assert(result == FWK_E_PARAM);
    is_valid_entity_id_return_val = true;

    /* Invalid event ID */
    is_valid_event_id_return_val = false;
    result = fwk_thread_put_event(&event2);
    assert(result == FWK_E_PARAM);
    is_valid_event_id_return_val = true;

    /* Incompatible target and event identifier */
    event2.id = FWK_ID_EVENT(0x2, 7);
    result = fwk_thread_put_event(&event2);
    assert(result == FWK_E_PARAM);

    result = fwk_thread_put_event(&event1);
    assert(result == FWK_SUCCESS);
    result_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->event_queue),
        struct fwk_event, slist_node);
    assert(fwk_id_is_equal(result_event->source_id, event1.source_id));
    assert(fwk_id_is_equal(result_event->target_id, event1.target_id));
    assert(result_event->is_response == event1.is_response);
    assert(result_event->response_requested == event1.response_requested);
    assert(result_event->is_notification == false);

    event2.id = FWK_ID_EVENT(0x3, 7);
    interrupt_get_current_return_val = FWK_SUCCESS;
    result = fwk_thread_put_event(&event2);
    assert(result == FWK_SUCCESS);
    assert(fwk_list_is_empty(&ctx->free_event_queue));
    result_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->isr_event_queue),
        struct fwk_event, slist_node);
    assert(fwk_id_is_equal(result_event->source_id, event2.source_id));
    assert(fwk_id_is_equal(result_event->target_id, event2.target_id));
    assert(result_event->is_response == true);
    assert(result_event->response_requested == event2.response_requested);
    assert(result_event->is_notification == false);
}

static void test___fwk_thread_put_notification(void)
{
    int result;
    struct fwk_event *result_event;

    struct fwk_event event1 = {
        .source_id = FWK_ID_MODULE(0x1),
        .target_id = FWK_ID_MODULE(0x2),
        .is_response = false,
        .response_requested = true,
        .id = FWK_ID_EVENT(0x2, 7),
    };

    struct fwk_event event2 = {
        .source_id = FWK_ID_MODULE(0x3),
        .target_id = FWK_ID_MODULE(0x4),
        .is_response = true,
        .response_requested = false,
    };

    result = __fwk_thread_init(2);
    assert(result == FWK_SUCCESS);

    result = __fwk_thread_put_notification(&event1);
    assert(result == FWK_SUCCESS);
    result_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->event_queue),
        struct fwk_event, slist_node);
    assert(fwk_id_is_equal(result_event->source_id, event1.source_id));
    assert(fwk_id_is_equal(result_event->target_id, event1.target_id));
    assert(result_event->is_response == false);
    assert(result_event->response_requested == event1.response_requested);
    assert(result_event->is_notification == true);

    event2.id = FWK_ID_EVENT(0x4, 7);
    interrupt_get_current_return_val = FWK_SUCCESS;
    result = __fwk_thread_put_notification(&event2);
    assert(result == FWK_SUCCESS);
    assert(fwk_list_is_empty(&ctx->free_event_queue));
    result_event = FWK_LIST_GET(fwk_list_pop_head(&ctx->isr_event_queue),
        struct fwk_event, slist_node);
    assert(fwk_id_is_equal(result_event->source_id, event2.source_id));
    assert(fwk_id_is_equal(result_event->target_id, event2.target_id));
    assert(result_event->is_response == false);
    assert(result_event->response_requested == event2.response_requested);
    assert(result_event->is_notification == true);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test___fwk_thread_init),
    FWK_TEST_CASE(test___fwk_thread_run),
    FWK_TEST_CASE(test_fwk_thread_put_event),
    FWK_TEST_CASE(test___fwk_thread_put_notification)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_thread",
    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,
    .test_case_teardown = test_case_teardown,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
