/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <internal/fwk_interrupt.h>

#include <fwk_arch.h>
#include <fwk_assert.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <stdbool.h>
#include <stdint.h>

#define INTERRUPT_ID 42

/*
 * Variables for the mock functions
 */
static int is_enabled_return_val;
static int enable_return_val;
static int disable_return_val;
static int is_pending_return_val;
static int set_pending_return_val;
static int clear_pending_return_val;
static int set_isr_return_val;
static int set_isr_param_return_val;
static int set_isr_nmi_return_val;
static int set_isr_nmi_param_return_val;
static int set_isr_fault_return_val;
static int get_current_return_val;

static void fake_isr(void)
{
    return;
}

static void fake_isr_param(uintptr_t param)
{
    return;
}

static int global_enable(void)
{
    return FWK_SUCCESS;
}

static int global_disable(void)
{
    return FWK_SUCCESS;
}

static int is_enabled(unsigned int interrupt, bool *state)
{
    return is_enabled_return_val;
}

static int enable(unsigned int interrupt)
{
    return enable_return_val;
}

static int disable(unsigned int interrupt)
{
    return disable_return_val;
}

static int is_pending(unsigned int interrupt, bool *state)
{
    return is_pending_return_val;
}

static int set_pending(unsigned int interrupt)
{
    return set_pending_return_val;
}

static int clear_pending(unsigned int interrupt)
{
    return clear_pending_return_val;
}

static int set_isr(unsigned int interrupt, void (*isr)(void))
{
    return set_isr_return_val;
}

static int set_isr_param(unsigned int interrupt,
                         void (*isr)(uintptr_t p),
                         uintptr_t p)
{
    return set_isr_param_return_val;
}

static int set_isr_nmi(void (*isr)(void))
{
    return set_isr_nmi_return_val;
}

static int set_isr_nmi_param(void (*isr)(uintptr_t p), uintptr_t p)
{
    return set_isr_nmi_param_return_val;
}

static int set_isr_fault(void (*isr)(void))
{
    return set_isr_fault_return_val;
}

static int get_current(unsigned int *interrupt)
{
    return get_current_return_val;
}

static bool is_interrupt_context(void)
{
    return (get_current_return_val == FWK_SUCCESS);
}

static const struct fwk_arch_interrupt_driver driver = {
    .global_enable = global_enable,
    .global_disable = global_disable,
    .is_enabled = is_enabled,
    .enable = enable,
    .disable = disable,
    .is_pending = is_pending,
    .set_pending = set_pending,
    .clear_pending = clear_pending,
    .set_isr_irq = set_isr,
    .set_isr_irq_param = set_isr_param,
    .set_isr_nmi = set_isr_nmi,
    .set_isr_nmi_param = set_isr_nmi_param,
    .set_isr_fault = set_isr_fault,
    .get_current = get_current,
    .is_interrupt_context = is_interrupt_context,
};

static const struct fwk_arch_interrupt_driver driver_invalid = {};

static void test_case_setup(void)
{
    is_enabled_return_val = FWK_E_HANDLER;
    enable_return_val = FWK_E_HANDLER;
    disable_return_val = FWK_E_HANDLER;
    is_pending_return_val = FWK_E_HANDLER;
    set_pending_return_val = FWK_E_HANDLER;
    clear_pending_return_val = FWK_E_HANDLER;
    set_isr_return_val = FWK_E_HANDLER;
    set_isr_param_return_val = FWK_E_HANDLER;
    set_isr_nmi_return_val = FWK_E_HANDLER;
    set_isr_nmi_param_return_val = FWK_E_HANDLER;
    set_isr_fault_return_val = FWK_E_HANDLER;
    get_current_return_val = FWK_E_HANDLER;
}

static void test_fwk_interrupt_before_init(void)
{
    int result;
    unsigned int interrupt = 1;
    bool state;

    result = fwk_interrupt_is_enabled(interrupt, &state);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_enable(interrupt);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_disable(interrupt);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_is_pending(interrupt, &state);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_set_pending(interrupt);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_clear_pending(interrupt);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_set_isr(interrupt, fake_isr);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_set_isr_param(interrupt, fake_isr_param, 0);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_set_isr_fault(fake_isr);
    assert(result == FWK_E_INIT);

    result = fwk_interrupt_get_current(&interrupt);
    assert(result == FWK_E_INIT);

    state = fwk_is_interrupt_context();
    assert(state == false);
}

static void test_fwk_interrupt_init(void)
{
    int result;

    result = fwk_interrupt_init(NULL);
    assert(result == FWK_E_PARAM);

    /* Driver has NULL value */
    result = fwk_interrupt_init(&driver_invalid);
    assert(result == FWK_E_PARAM);

    result = fwk_interrupt_init(&driver);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_critical_section(void)
{
    unsigned int flags;

    flags = fwk_interrupt_global_disable();
    assert(critical_section_nest_level == 1);

    fwk_interrupt_global_enable(flags);
    assert(critical_section_nest_level == 0);
}

static void test_fwk_interrupt_is_enabled(void)
{
    bool state;
    int result;

    result = fwk_interrupt_is_enabled(INTERRUPT_ID, NULL);
    assert(result == FWK_E_PARAM);

    is_enabled_return_val = FWK_SUCCESS;
    result = fwk_interrupt_is_enabled(INTERRUPT_ID, &state);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_enable(void)
{
    int result;

    enable_return_val = FWK_SUCCESS;
    result = fwk_interrupt_enable(INTERRUPT_ID);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_disable(void)
{
    int result;

    disable_return_val = FWK_SUCCESS;
    result = fwk_interrupt_disable(INTERRUPT_ID);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_is_pending(void)
{
    int result;
    bool state;

    result = fwk_interrupt_is_pending(INTERRUPT_ID, NULL);
    assert(result == FWK_E_PARAM);

    is_pending_return_val = FWK_SUCCESS;
    result = fwk_interrupt_is_pending(INTERRUPT_ID, &state);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_set_pending(void)
{
    int result;

    set_pending_return_val = FWK_SUCCESS;
    result = fwk_interrupt_set_pending(INTERRUPT_ID);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_clear_pending(void)
{
    int result;

    clear_pending_return_val = FWK_SUCCESS;
    result = fwk_interrupt_clear_pending(INTERRUPT_ID);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_set_isr(void)
{
    int result;

    result = fwk_interrupt_set_isr(INTERRUPT_ID, NULL);
    assert(result == FWK_E_PARAM);

    set_isr_return_val = FWK_SUCCESS;
    result = fwk_interrupt_set_isr(INTERRUPT_ID, fake_isr);
    assert(result == FWK_SUCCESS);

    set_isr_return_val = FWK_E_HANDLER;
    set_isr_nmi_return_val = FWK_SUCCESS;
    result = fwk_interrupt_set_isr(FWK_INTERRUPT_NMI, fake_isr);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_set_isr_param(void)
{
    int result;

    result = fwk_interrupt_set_isr_param(INTERRUPT_ID, NULL, 0);
    assert(result == FWK_E_PARAM);

    set_isr_param_return_val = FWK_SUCCESS;
    result = fwk_interrupt_set_isr_param(INTERRUPT_ID, fake_isr_param, 0);
    assert(result == FWK_SUCCESS);

    set_isr_param_return_val = FWK_E_HANDLER;
    set_isr_nmi_param_return_val = FWK_SUCCESS;
    result = fwk_interrupt_set_isr_param(FWK_INTERRUPT_NMI, fake_isr_param, 0);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_set_isr_fault(void)
{
    int result;

    result = fwk_interrupt_set_isr_fault(NULL);
    assert(result == FWK_E_PARAM);

    set_isr_fault_return_val = FWK_SUCCESS;
    result = fwk_interrupt_set_isr_fault(fake_isr);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_get_current(void)
{
    int result;
    unsigned int interrupt;

    result = fwk_interrupt_get_current(NULL);
    assert(result == FWK_E_PARAM);

    get_current_return_val = FWK_SUCCESS;
    result = fwk_interrupt_get_current(&interrupt);
    assert(result == FWK_SUCCESS);
}

static void test_fwk_interrupt_nested_critical_section(void)
{
    unsigned int flags1, flags2, flags3;
    flags1 = fwk_interrupt_global_disable();
    assert(critical_section_nest_level == 1);

    flags2 = fwk_interrupt_global_disable();
    assert(critical_section_nest_level == 2);

    flags3 = fwk_interrupt_global_disable();
    assert(critical_section_nest_level == 3);

    fwk_interrupt_global_enable(flags3);
    assert(critical_section_nest_level == 2);

    fwk_interrupt_global_enable(flags2);
    assert(critical_section_nest_level == 1);

    fwk_interrupt_global_enable(flags1);
    assert(critical_section_nest_level == 0);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_interrupt_before_init),
    FWK_TEST_CASE(test_fwk_interrupt_init),
    FWK_TEST_CASE(test_fwk_interrupt_critical_section),
    FWK_TEST_CASE(test_fwk_interrupt_is_enabled),
    FWK_TEST_CASE(test_fwk_interrupt_enable),
    FWK_TEST_CASE(test_fwk_interrupt_disable),
    FWK_TEST_CASE(test_fwk_interrupt_is_pending),
    FWK_TEST_CASE(test_fwk_interrupt_set_pending),
    FWK_TEST_CASE(test_fwk_interrupt_clear_pending),
    FWK_TEST_CASE(test_fwk_interrupt_set_isr),
    FWK_TEST_CASE(test_fwk_interrupt_set_isr_param),
    FWK_TEST_CASE(test_fwk_interrupt_set_isr_fault),
    FWK_TEST_CASE(test_fwk_interrupt_get_current),
    FWK_TEST_CASE(test_fwk_interrupt_nested_critical_section),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_interrupt",
    .test_case_setup = test_case_setup,
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
