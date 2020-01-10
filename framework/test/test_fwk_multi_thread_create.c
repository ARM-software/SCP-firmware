
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rtx_os.h>

#include <internal/fwk_id.h>
#include <internal/fwk_module.h>
#include <internal/fwk_multi_thread.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ELEM_THREAD_ID      10
#define MODULE_THREAD_ID    11

struct __fwk_multi_thread_ctx *fwk_multi_thread_ctx;
struct fwk_slist_node slist_node[4];

/* Mock module and element */
static struct __fwk_thread_ctx fake_thread_element_ctx;
static struct fwk_element fake_element;
static struct fwk_element_ctx fake_element_ctx;
static struct __fwk_thread_ctx fake_thread_module_ctx;
static struct fwk_module fake_module;
static struct fwk_module_ctx fake_module_ctx;

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
    (void) flags;
    (void) options;
    (void) timeout;
    return flags;
}

uint32_t __wrap_osThreadFlagsClear(uint32_t flags)
{
    (void)flags;
    return 0;
}

uint32_t __wrap_osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    (void) thread_id;
    (void) flags;
    return 0;
}

static osThreadId_t osThreadNew_return_val;
static const osThreadAttr_t *osThreadNew_param_attr;
osThreadId_t __wrap_osThreadNew(osThreadFunc_t func, void *argument,
                                const osThreadAttr_t *attr)
{
    (void) func;
    (void) argument;
    osThreadNew_param_attr = attr;
    return osThreadNew_return_val;
}

static int fwk_interrupt_get_current_return_val;
int __wrap_fwk_interrupt_get_current(void)
{
    return fwk_interrupt_get_current_return_val;
}

/* Wrapped framework functions */
static unsigned int fwk_mm_calloc_return_null;
static unsigned int fwk_mm_calloc_call_count;
static void *fwk_mm_calloc_val;
void *__wrap_fwk_mm_calloc(size_t num, size_t size)
{
    fwk_mm_calloc_call_count++;
    if (fwk_mm_calloc_return_null == fwk_mm_calloc_call_count)
        fwk_mm_calloc_val = NULL;
    else
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
    (void) state;
    return FWK_SUCCESS;
}

static int test_suite_setup(void)
{
    fwk_multi_thread_ctx = __fwk_multi_thread_get_ctx();

    return FWK_SUCCESS;
}

static void test_case_setup(void)
{

    fwk_mm_calloc_return_null = UINT_MAX;
    fwk_mm_calloc_call_count = 0;
    fwk_interrupt_get_current_return_val = FWK_SUCCESS;
    osThreadNew_return_val = (void*)MODULE_THREAD_ID;

    fake_thread_element_ctx.os_thread_id = (osThreadId_t)ELEM_THREAD_ID;
    fake_thread_module_ctx.os_thread_id = (osThreadId_t)MODULE_THREAD_ID;
    fake_module_ctx = (struct fwk_module_ctx){ };
    fake_module_ctx.desc = &fake_module;
    fake_module_ctx.thread_ctx = NULL;

    fake_element_ctx = (struct fwk_element_ctx){ };
    fake_element_ctx.desc = &fake_element;
    fake_element_ctx.thread_ctx = NULL;

    fwk_module_is_valid_element_id_return_val = false;
    fwk_module_is_valid_module_id_return_val = true;
}

static void test_create_common_thread(void)
{
    int result;

    result = __fwk_thread_init(16);
    assert(result == FWK_SUCCESS);
}

static void test_create_id_invalid(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* The ID is invalid */
    fwk_module_is_valid_module_id_return_val = false;
    status = fwk_thread_create(id);
    assert(status == FWK_E_PARAM);

}

static void test_create_not_initialized(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* Thread component not initialized */
    fwk_multi_thread_ctx->running = true;
    status = fwk_thread_create(id);
    assert(status == FWK_E_STATE);
    fwk_multi_thread_ctx->running = false;
}

static void test_create_thread_invalid(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* module thread ctx is not null */
    fake_module_ctx.thread_ctx = &fake_thread_module_ctx;
    status = fwk_thread_create(id);
    assert(status == FWK_E_STATE);
}

static void test_create_thread_memory_allocation_failed(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* Thread memory allocation failed */
    fwk_mm_calloc_return_null = 3;
    status = fwk_thread_create(id);
    assert(status == FWK_E_NOMEM);
}

static void test_create_thread_creation_failed(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* Thread creation failed */
    osThreadNew_return_val = NULL;
    status = fwk_thread_create(id);
    assert(status == FWK_E_OS);
}

static void test_create_element_thread(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* Thread creation for an element */
    fwk_module_is_valid_element_id_return_val = true;
    status = fwk_thread_create(id);
    assert(status == FWK_SUCCESS);
    assert(fake_element_ctx.thread_ctx == fwk_mm_calloc_val);
}

static void test_create_module_thread(void)
{
    int status;
    fwk_id_t id = FWK_ID_MODULE(0x1);

    /* Thread creation for a module */
    status = fwk_thread_create(id);
    assert(status == FWK_SUCCESS);
    assert(fake_module_ctx.thread_ctx == fwk_mm_calloc_val);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_create_common_thread),
    FWK_TEST_CASE(test_create_id_invalid),
    FWK_TEST_CASE(test_create_not_initialized),
    FWK_TEST_CASE(test_create_thread_invalid),
    FWK_TEST_CASE(test_create_thread_memory_allocation_failed),
    FWK_TEST_CASE(test_create_thread_creation_failed),
    FWK_TEST_CASE(test_create_element_thread),
    FWK_TEST_CASE(test_create_module_thread)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_multi_thread_create",
    .test_suite_setup = test_suite_setup,
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
