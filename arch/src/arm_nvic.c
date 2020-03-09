/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interrupt management.
 */

#include <cmsis_compiler.h>

#include <fwk_arch.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef BUILD_HAS_MULTITHREADING
#    include <rtx_os.h>
#endif

extern noreturn void arm_exception_invalid(void);

#define SCB_SHCSR ((FWK_RW uint32_t *)(0xE000ED24UL))
#define SCB_VTOR ((FWK_RW uint32_t *)(0xE000ED08UL))
#define SCS_ICTR ((FWK_R  uint32_t *)(0xE000E004UL))
#define SCS_STIR ((FWK_W  uint32_t *)(0xE000EF00UL))
#define SCS_NVIC ((struct nvic *)(0xE000E100UL))

#define SCB_SHCSR_MEMFAULTENA_MASK  (1 << 16)
#define SCB_SHCSR_BUSFAULTENA_MASK  (1 << 17)
#define SCB_SHCSR_USGFAULTENA_MASK  (1 << 18)

enum exception_num {
    EXCEPTION_NUM_INVALID      = 0U,
    EXCEPTION_NUM_RESET        = 1U,
    EXCEPTION_NUM_NMI          = 2U,
    EXCEPTION_NUM_HARDFAULT    = 3U,
    EXCEPTION_NUM_MEMMANAGE    = 4U,
    EXCEPTION_NUM_BUSFAULT     = 5U,
    EXCEPTION_NUM_USAGEFAULT   = 6U,
    EXCEPTION_NUM_SVCALL       = 11U,
    EXCEPTION_NUM_DEBUGMONITOR = 12U,
    EXCEPTION_NUM_PENDSV       = 14U,
    EXCEPTION_NUM_SYSTICK      = 15U,
    EXCEPTION_NUM_COUNT,
};

struct nvic {
    FWK_RW uint32_t ISER[16];      /* Interrupt Set Enabled Register */
           uint32_t RESERVED0[16];
    FWK_RW uint32_t ICER[16];      /* Interrupt Clear Enabled Register */
           uint32_t RESERVED1[16];
    FWK_RW uint32_t ISPR[16];      /* Interrupt Set Pending Register  */
           uint32_t RESERVED2[16];
    FWK_RW uint32_t ICPR[16];      /* Interrupt Clear Pending Register */
           uint32_t RESERVED3[16];
    FWK_R  uint32_t IABR[16];      /* Interrupt Active Bit Register */
           uint32_t RESERVED4[48];
    FWK_W  uint32_t IPR[16];       /* Interrupt Priority Register */
           uint32_t RESERVED5[48];
};

static uint32_t isr_count;
static uint32_t irq_count;

/*
 * For interrupts with parameters, their entry in the vector table points to a
 * global handler that calls a registered function in the callback table with a
 * corresponding parameter. Entries in the vector table for interrupts without
 * parameters point directly to the handler functions.
 */
struct callback {
    void (*func)(uintptr_t param);
    uintptr_t param;
};

static void (**vector)(void);
static struct callback *callback;

static void irq_global(void)
{
    struct callback *entry = &callback[__get_IPSR()];
    entry->func(entry->param);
}

static int global_enable(void)
{
    __enable_irq();

    return FWK_SUCCESS;
}

static int global_disable(void)
{
    __disable_irq();

    return FWK_SUCCESS;
}

static int is_enabled(unsigned int interrupt, bool *enabled)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    *enabled = SCS_NVIC->ISER[interrupt / 32] &
        (UINT32_C(1) << (interrupt & 0x1F));

    return FWK_SUCCESS;
}

static int enable(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    SCS_NVIC->ISER[interrupt / 32] = UINT32_C(1) << (interrupt & 0x1F);

    return FWK_SUCCESS;
}

static int disable(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    SCS_NVIC->ICER[interrupt / 32] = UINT32_C(1) << (interrupt & 0x1F);

    return FWK_SUCCESS;
}

static int is_pending(unsigned int interrupt, bool *pending)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    *pending = SCS_NVIC->ISPR[interrupt / 32] &
        (UINT32_C(1) << (interrupt & 0x1F));

    return FWK_SUCCESS;
}

static int set_pending(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    *SCS_STIR = interrupt;

    return FWK_SUCCESS;
}

static int clear_pending(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    SCS_NVIC->ICPR[interrupt / 32] = UINT32_C(1) << (interrupt & 0x1F);

    return FWK_SUCCESS;
}

static int set_isr_irq(unsigned int interrupt, void (*isr)(void))
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    vector[EXCEPTION_NUM_COUNT + interrupt] = isr;

    return FWK_SUCCESS;
}

static int set_isr_irq_param(unsigned int interrupt,
                             void (*isr)(uintptr_t param),
                             uintptr_t parameter)
{
    struct callback *entry;
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    vector[EXCEPTION_NUM_COUNT + interrupt] = irq_global;

    entry = &callback[EXCEPTION_NUM_COUNT + interrupt];
    entry->func = isr;
    entry->param = parameter;

    return FWK_SUCCESS;
}

static int set_isr_nmi(void (*isr)(void))
{
    vector[EXCEPTION_NUM_NMI] = isr;

    return FWK_SUCCESS;
}

static int set_isr_nmi_param(void (*isr)(uintptr_t param), uintptr_t parameter)
{
    struct callback *entry;

    vector[EXCEPTION_NUM_NMI] = irq_global;

    entry = &callback[EXCEPTION_NUM_NMI];
    entry->func = isr;
    entry->param = parameter;

    return FWK_SUCCESS;
}

static int set_isr_fault(void (*isr)(void))
{
    vector[EXCEPTION_NUM_HARDFAULT]  = isr;
    vector[EXCEPTION_NUM_MEMMANAGE]  = isr;
    vector[EXCEPTION_NUM_BUSFAULT]   = isr;
    vector[EXCEPTION_NUM_USAGEFAULT] = isr;

    return FWK_SUCCESS;
}

static int get_current(unsigned int *interrupt)
{
    *interrupt = __get_IPSR();

    /* Not an interrupt */
    if (*interrupt == 0)
        return FWK_E_STATE;

    if (*interrupt == EXCEPTION_NUM_NMI)
        *interrupt = FWK_INTERRUPT_NMI;
    else if ((*interrupt) < EXCEPTION_NUM_COUNT)
        *interrupt = FWK_INTERRUPT_EXCEPTION;
    else
        *interrupt -= EXCEPTION_NUM_COUNT;

    return FWK_SUCCESS;
}

static const struct fwk_arch_interrupt_driver arm_nvic_driver = {
    .global_enable     = global_enable,
    .global_disable    = global_disable,
    .is_enabled        = is_enabled,
    .enable            = enable,
    .disable           = disable,
    .is_pending        = is_pending,
    .set_pending       = set_pending,
    .clear_pending     = clear_pending,
    .set_isr_irq       = set_isr_irq,
    .set_isr_irq_param = set_isr_irq_param,
    .set_isr_nmi       = set_isr_nmi,
    .set_isr_nmi_param = set_isr_nmi_param,
    .set_isr_fault     = set_isr_fault,
    .get_current       = get_current,
};

static void irq_invalid(void)
{
    disable(__get_IPSR());
}

int arm_nvic_init(const struct fwk_arch_interrupt_driver **driver)
{
    uint32_t ictr_intlinesnum;
    uint32_t align_entries;
    uint32_t align_word;
    unsigned int i;

    if (driver == NULL)
        return FWK_E_PARAM;

    /* Find the number of interrupt lines implemented in hardware */
    ictr_intlinesnum = *SCS_ICTR & 0x0000000F;
    irq_count = (ictr_intlinesnum + 1) * 32;
    isr_count = irq_count + EXCEPTION_NUM_COUNT;

    /*
     * Allocate and initialize a table for the callback functions and their
     * corresponding parameters.
     */
    callback = fwk_mm_calloc(isr_count, sizeof(callback[0]));

    /*
     * The base address for the vector table must align on the number of
     * entries in the table, corresponding to a word boundary rounded up to the
     * next power of two.
     *
     * For example, for a vector table with 48 entries, the base address must be
     * on a 64-word boundary.
     */

    /* Calculate the next power of two */
    align_entries = UINT32_C(1) << (32 - __CLZ(isr_count - 1));

    /* Calculate alignment on a word boundary */
    align_word = align_entries * sizeof(vector[0]);

    vector = fwk_mm_alloc_aligned(isr_count, sizeof(vector[0]), align_word);

    /*
     * Initialize all exception entries to point to the arm_exception_invalid()
     * handler.
     *
     * Note: Initialization starts from entry 1 since entry 0 is not an
     * exception pointer but the default stack pointer.
     */
    for (i = 1; i < EXCEPTION_NUM_COUNT; i++)
        vector[i] = arm_exception_invalid;

    /* Initialize IRQs */
    for (i = 0; i < irq_count; i++) {
        /* Initialize all IRQ entries to point to the irq_invalid() handler */
        vector[EXCEPTION_NUM_COUNT + i] = irq_invalid;

        /* Ensure IRQs are disabled during boot sequence */
        disable(i);
        clear_pending(i);
    }

#ifdef BUILD_HAS_MULTITHREADING
    /* Set exception entries that are implemented and handled by RTX */
    vector[EXCEPTION_NUM_SVCALL] = SVC_Handler;
    vector[EXCEPTION_NUM_PENDSV] = PendSV_Handler;
    vector[EXCEPTION_NUM_SYSTICK] = SysTick_Handler;
#endif

    __DMB();

    /* Switch to the new vector table */
    *SCB_VTOR = (uint32_t)vector;
    __DMB();

    /* Enable the Usage, Bus and Memory faults which are disabled by default */
    *SCB_SHCSR |= SCB_SHCSR_MEMFAULTENA_MASK |
                  SCB_SHCSR_BUSFAULTENA_MASK |
                  SCB_SHCSR_USGFAULTENA_MASK;

    *driver = &arm_nvic_driver;

    return FWK_SUCCESS;
}
