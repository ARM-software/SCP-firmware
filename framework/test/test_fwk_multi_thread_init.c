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
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define COMMON_THREAD_ID    12

/* Wrapped OS functions */

osStatus_t __wrap_osKernelStart(void)
{
    return osOK;
}

static osStatus_t osKernelInitailize_return_val;
osStatus_t __wrap_osKernelInitialize(void)
{
    return osKernelInitailize_return_val;
}

/*
 * Indicate the number of function calls before exiting the non-returning
 * functions
 */
uint32_t __wrap_osThreadFlagsWait(uint32_t flags, uint32_t options,
                                  uint32_t timeout)
{
    (void) flags;
    (void) options;
    (void) timeout;
    return 0;
}

uint32_t __wrap_osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    (void) thread_id;
    (void) flags;
    return 0;
}

uint32_t __wrap_osThreadFlagsClear(uint32_t flags)
{
    (void)flags;
    return 0;
}

static osThreadId_t osThreadNew_return_val;
osThreadId_t __wrap_osThreadNew(osThreadFunc_t func, void *argument,
                                const osThreadAttr_t *attr)
{
    (void) func;
    (void) argument;
    (void) attr;
    return osThreadNew_return_val;
}

int __wrap_fwk_interrupt_get_current(unsigned int *interrupt)
{
    (void)interrupt;
    return FWK_SUCCESS;
}

/* Wrapped framework functions */
/* Indicate after how many calls the calloc function failed */
static unsigned int fwk_mm_calloc_failed_call;
static unsigned int fwk_mm_calloc_count_call;
static void *fwk_mm_calloc_val;
void *__wrap_fwk_mm_calloc(size_t num, size_t size)
{
    if (fwk_mm_calloc_failed_call == fwk_mm_calloc_count_call)
        fwk_mm_calloc_val = NULL;
    else
        fwk_mm_calloc_val = (void *)calloc(num, size);

    fwk_mm_calloc_count_call++;
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
    return false;
}

bool __wrap_fwk_module_is_valid_entity_id(fwk_id_t id)
{
    return false;
}

bool __wrap_fwk_module_is_valid_event_id(fwk_id_t id)
{
    return false;
}

bool __wrap_fwk_module_is_valid_notification_id(fwk_id_t id)
{
    return false;
}

struct fwk_element_ctx *__wrap___fwk_module_get_element_ctx(fwk_id_t id)
{
    (void) id;
    return NULL;
}

struct fwk_module_ctx *__wrap___fwk_module_get_ctx(fwk_id_t id)
{
    (void) id;
    return NULL;
}

int __wrap___fwk_module_get_state(fwk_id_t id, enum fwk_module_state *state)
{
    (void) id;
    (void) state;
    return FWK_SUCCESS;
}

static struct __fwk_multi_thread_ctx *ctx;

static int test_suite_setup(void)
{
    ctx = __fwk_multi_thread_get_ctx();

    return FWK_SUCCESS;
}

static void test_case_setup(void)
{

    fwk_mm_calloc_failed_call = 3;
    fwk_mm_calloc_count_call = 0;
    osThreadNew_return_val = (osThreadId_t)COMMON_THREAD_ID;
    osKernelInitailize_return_val = osOK;
}

static void test_init_common_thread_failed(void)
{
    int result;

    osThreadNew_return_val = NULL;
    /* Creation of the common thread failed */
    result = __fwk_thread_init(2);
    assert(result == FWK_E_OS);
}

static void test_init_succeed(void)
{
    int result;

    /* Insert 2 events in the list */
    result = __fwk_thread_init(2);
    assert(result == FWK_SUCCESS);
    assert(!fwk_list_is_empty(&ctx->event_free_queue));
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_init_common_thread_failed),
    FWK_TEST_CASE(test_init_succeed),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_multi_thread_init",
    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
