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
#include <stdlib.h>

#define ELEM_THREAD_ID      10
#define MODULE_THREAD_ID    11
#define COMMON_THREAD_ID    12

#define SIGNAL_EVENT_TO_PROCESS 0x02

struct fwk_slist_node slist_node[4];
struct fwk_event event[4];
struct __fwk_multi_thread_ctx *fwk_multi_thread_ctx;

/* Mock module and element */
static struct __fwk_thread_ctx fake_thread_element_ctx;
static struct fwk_element fake_element;
static struct fwk_element_ctx fake_element_ctx;
static struct __fwk_thread_ctx fake_thread_module_ctx;
static struct fwk_module fake_module;
static struct fwk_module_ctx fake_module_ctx;

static int process_event(const struct fwk_event *evt,
                         struct fwk_event *response_event)
{
    (void) evt;
    (void) response_event;
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
uint32_t __wrap_osThreadFlagsWait(uint32_t flags, uint32_t options,
                                  uint32_t timeout)
{
    return 0;
}

uint32_t __wrap_osThreadFlagsClear(uint32_t flags)
{
    return 0;
}

uint32_t __wrap_osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    return -1;
}

static osThreadFunc_t specific_thread_function;
osThreadId_t __wrap_osThreadNew(osThreadFunc_t func, void *argument,
                                const osThreadAttr_t *attr)
{
    specific_thread_function = func;
    return (osThreadId_t) ELEM_THREAD_ID;
}

static int fwk_interrupt_get_current_return_val;
int __wrap_fwk_interrupt_get_current(unsigned int *interrupt)
{
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

static bool fwk_module_is_valid_element_id_return_val;
bool __wrap_fwk_module_is_valid_element_id(fwk_id_t id)
{
    return fwk_module_is_valid_element_id_return_val;
}

static bool fwk_module_is_valid_module_id_return_val;
bool __wrap_fwk_module_is_valid_module_id(fwk_id_t id)
{
    return fwk_module_is_valid_module_id_return_val;
}

bool __wrap_fwk_module_is_valid_entity_id(fwk_id_t id)
{
    return true;
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

static struct __fwk_multi_thread_ctx *ctx;

static int test_suite_setup(void)
{
    int status;

    ctx = __fwk_multi_thread_get_ctx();
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    status = __fwk_thread_init(2);
    if (status != FWK_SUCCESS)
        return status;

    fwk_module_is_valid_module_id_return_val = true;
    status = fwk_thread_create(FWK_ID_MODULE(0x1));
    if (status != FWK_SUCCESS)
        return status;

    fwk_multi_thread_ctx = __fwk_multi_thread_get_ctx();

    return FWK_SUCCESS;
}

static void test_case_setup(void)
{
    fwk_multi_thread_ctx->waiting_for_isr_event = false;
    fwk_multi_thread_ctx->running = false;
    fwk_multi_thread_ctx->current_thread_ctx = NULL;

    event[0].slist_node = slist_node[0];
    event[0].source_id = FWK_ID_MODULE(0x1);
    event[0].target_id = FWK_ID_MODULE(0x2);
    event[0].cookie = 0;
    event[0].is_response = false;
    event[0].response_requested = false;
    event[0].id = FWK_ID_EVENT(2, 7);

    event[1].slist_node = slist_node[1];
    event[1].source_id = FWK_ID_MODULE(0x3);
    event[1].target_id = FWK_ID_MODULE(0x4);
    event[1].cookie = 0;
    event[1].is_response = false;
    event[1].response_requested = false;
    event[1].id = FWK_ID_EVENT(4, 8);

    event[2].slist_node = slist_node[2];
    event[2].source_id = FWK_ID_MODULE(0x5);
    event[2].target_id = FWK_ID_MODULE(0x6);
    event[2].cookie = 0;
    event[2].is_response = false;
    event[2].response_requested = true;
    event[2].id = FWK_ID_EVENT(6, 9);

    event[3].slist_node = slist_node[3];
    event[3].source_id = FWK_ID_MODULE(0x7);
    event[3].target_id = FWK_ID_MODULE(0x8);
    event[3].cookie = 0;
    event[3].is_response = false;
    event[3].response_requested = false;
    event[3].id = FWK_ID_EVENT(8, 10);

    fwk_interrupt_get_current_return_val = FWK_SUCCESS;

    fake_thread_element_ctx.os_thread_id = (osThreadId_t)ELEM_THREAD_ID;
    fake_thread_module_ctx.os_thread_id = (osThreadId_t)MODULE_THREAD_ID;
    fake_module.process_event = process_event;
    fake_module_ctx.desc = &fake_module;
    fake_module_ctx.thread_ctx = &fake_thread_module_ctx;
    fake_module_ctx.thread_ctx->response_event = NULL;

    fake_element_ctx.desc = &fake_element;
    fake_element_ctx.thread_ctx = &fake_thread_element_ctx;

    fwk_module_is_valid_element_id_return_val = false;
    fwk_module_is_valid_module_id_return_val = true;

    fwk_list_init(&ctx->event_free_queue);
    fwk_list_init(&ctx->thread_ready_queue);
    fwk_list_init(&ctx->event_isr_queue);
    fwk_list_init(&ctx->common_thread_ctx.event_queue);
    fwk_list_init(&fake_thread_module_ctx.event_queue);
    fwk_list_init(&fake_thread_element_ctx.event_queue);
}

static void test_put_event_thread_ctx_in_thread_ready_queue(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fake_module_ctx.thread_ctx->waiting_event_processing_completion = false;
    /* thread context added to thread_ready_queue */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[1].slist_node);
    assert(ctx->thread_ready_queue.head ==
        &fake_module_ctx.thread_ctx->slist_node);
    assert(fwk_list_is_empty(&ctx->event_free_queue));
}

static void test_put_event_not_empty_target_list(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fake_module_ctx.thread_ctx->waiting_event_processing_completion = false;
    /* thread context not added to thread_ready_queue */
    fwk_list_push_tail(&ctx->event_free_queue, &event[2].slist_node);
    fwk_list_push_tail(&fake_module_ctx.thread_ctx->event_queue,
        &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[2].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_free_queue));
}

static void test_put_event_waiting_on_completion(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fake_module_ctx.thread_ctx->waiting_event_processing_completion = true;
    /* thread context not added to thread_ready_queue */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx->event_queue.tail ==
        &event[1].slist_node);
    assert(fwk_list_is_empty(&ctx->thread_ready_queue));
    assert(fwk_list_is_empty(&ctx->event_free_queue));
}

static void test_thread_get_ctx_invalid_id(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_module_is_valid_module_id_return_val = false;
    /* Invalid ID */
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);
}

static void test_thread_get_ctx_common_thread(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fake_module_ctx.thread_ctx = NULL;
    /* Get common thread context */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(ctx->common_thread_ctx.event_queue.head == &event[1].slist_node);
}

static void test_thread_get_ctx_module_context(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    /* Get module context */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
}

static void test_thread_get_ctx_element_context(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_module_is_valid_element_id_return_val = true;
     /* Get element context */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fake_element_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
}

static void test_thread_get_ctx_module_from_element_id(void)
{
    int status;

    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_module_is_valid_element_id_return_val = true;
    fake_element_ctx.thread_ctx = NULL;
    /* Get module context from element ID */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx->event_queue.head ==
        &event[1].slist_node);
}
static void test_thread_get_ctx_invalid_module_from_element_id(void)
{
    int status;
    fwk_interrupt_get_current_return_val = FWK_E_STATE;
    fwk_module_is_valid_element_id_return_val = true;
    fwk_module_is_valid_module_id_return_val = false;
    fake_element_ctx.thread_ctx = NULL;
    /* Invalid ID */
    fwk_list_push_tail(&ctx->event_free_queue, &event[1].slist_node);
    status = fwk_thread_put_event(&event[0]);
    assert(status == FWK_E_PARAM);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_put_event_thread_ctx_in_thread_ready_queue),
    FWK_TEST_CASE(test_put_event_not_empty_target_list),
    FWK_TEST_CASE(test_put_event_waiting_on_completion),
    FWK_TEST_CASE(test_thread_get_ctx_invalid_id),
    FWK_TEST_CASE(test_thread_get_ctx_common_thread),
    FWK_TEST_CASE(test_thread_get_ctx_module_context),
    FWK_TEST_CASE(test_thread_get_ctx_element_context),
    FWK_TEST_CASE(test_thread_get_ctx_module_from_element_id),
    FWK_TEST_CASE(test_thread_get_ctx_invalid_module_from_element_id)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_multi_thread_util",
    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
