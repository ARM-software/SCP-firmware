/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <stdbool.h>

static int fwk_mm_init_return_val;
static int mm_init_handler_return_val;
static int fwk_interrupt_init_return_val;
static int interrupt_init_handler_return_val;
static int __fwk_module_init_return_val;

/*
 * Mock functions
 */
int __wrap_fwk_mm_init(uintptr_t start, size_t size)
{
    return fwk_mm_init_return_val;
}

int mm_init_handler(struct fwk_arch_mm_data *data)
{
    return mm_init_handler_return_val;
}

int __wrap_fwk_interrupt_init(const struct fwk_arch_interrupt_driver *driver)
{
    return fwk_interrupt_init_return_val;
}

int interrupt_init_handler(struct fwk_arch_interrupt_driver **driver)
{
    return interrupt_init_handler_return_val;
}

int __wrap___fwk_module_init(void)
{
    return __fwk_module_init_return_val;
}

static const struct fwk_arch_init_driver driver_invalid = {
    .mm = NULL,
};

static void test_case_setup(void)
{
    fwk_mm_init_return_val = FWK_SUCCESS;
    mm_init_handler_return_val = FWK_SUCCESS;
    fwk_interrupt_init_return_val = FWK_SUCCESS;
    interrupt_init_handler_return_val = FWK_SUCCESS;
    __fwk_module_init_return_val = FWK_SUCCESS;
}

static const struct fwk_arch_init_driver driver = {
    .mm = mm_init_handler,
    .interrupt = interrupt_init_handler,
};

static void test_fwk_arch_init_success(void)
{
    int result;

    result = fwk_arch_init(&driver);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_arch_init_bad_param(void)
{
    int result;

    result = fwk_arch_init(NULL);
    assert(result == FWK_E_PARAM);

    /* The driver contains a NULL value */
    result = fwk_arch_init(&driver_invalid);
    assert(result == FWK_E_PARAM);
}

static void test_fwk_arch_init_mm_fail(void)
{
    int result;

    /* Memory management initialization failed */
    fwk_mm_init_return_val = FWK_E_STATE;
    result = fwk_arch_init(&driver);
    assert(result == FWK_E_PANIC);

    /* Memory management handler failed */
    mm_init_handler_return_val = FWK_E_PARAM;
    result = fwk_arch_init(&driver);
    assert(result == FWK_E_PANIC);
}

static void test_fwk_arch_init_interrupt_fail(void)
{
    int result;

    /* Interrupt initialization failed */
    fwk_interrupt_init_return_val = FWK_E_PARAM;
    result = fwk_arch_init(&driver);
    assert(result == FWK_E_PANIC);

    /* Interrupt handler failed */
    interrupt_init_handler_return_val = FWK_E_PARAM;
    result = fwk_arch_init(&driver);
    assert(result == FWK_E_PANIC);
}

static void test_fwk_arch_init_module_fail(void)
{
    int result;

    /* Module initialization failed */
    __fwk_module_init_return_val = FWK_E_STATE;
    result = fwk_arch_init(&driver);
    assert(result == FWK_E_PANIC);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_arch_init_success),
    FWK_TEST_CASE(test_fwk_arch_init_bad_param),
    FWK_TEST_CASE(test_fwk_arch_init_mm_fail),
    FWK_TEST_CASE(test_fwk_arch_init_interrupt_fail),
    FWK_TEST_CASE(test_fwk_arch_init_module_fail)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_arch",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
