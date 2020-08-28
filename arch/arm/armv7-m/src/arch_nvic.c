/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interrupt management.
 */

#include <fwk_arch.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <arch_exceptions.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static unsigned int isr_count;
static unsigned int irq_count;

/*
 * For interrupts with parameters, their entry in the vector table points to a
 * global handler that calls a registered function in the callback table with a
 * corresponding parameter. Entries in the vector table for interrupts without
 * parameters point directly to the handler functions.
 *
 * Entry indices are offset by -1 relative to their interrupt numbers, as no
 * interrupt may have an interrupt number of zero.
 */
struct callback {
    void (*func)(uintptr_t param);
    uintptr_t param;
};

static struct callback *callback;

static void irq_global(void)
{
    struct callback *entry = &callback[__get_IPSR() - 1];

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

    *enabled = NVIC_GetEnableIRQ(interrupt);

    return FWK_SUCCESS;
}

static int enable(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    NVIC_EnableIRQ(interrupt);

    return FWK_SUCCESS;
}

static int disable(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    NVIC_DisableIRQ(interrupt);

    return FWK_SUCCESS;
}

static int is_pending(unsigned int interrupt, bool *pending)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    *pending = NVIC_GetPendingIRQ(interrupt);

    return FWK_SUCCESS;
}

static int set_pending(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    NVIC_SetPendingIRQ(interrupt);

    return FWK_SUCCESS;
}

static int clear_pending(unsigned int interrupt)
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    NVIC_ClearPendingIRQ(interrupt);

    return FWK_SUCCESS;
}

static int set_isr_irq(unsigned int interrupt, void (*isr)(void))
{
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    NVIC_SetVector(interrupt, (uint32_t)isr);

    return FWK_SUCCESS;
}

static int set_isr_irq_param(
    unsigned int interrupt,
    void (*isr)(uintptr_t param),
    uintptr_t parameter)
{
    struct callback *entry;
    if (interrupt >= irq_count)
        return FWK_E_PARAM;

    entry = &callback[NVIC_USER_IRQ_OFFSET + interrupt - 1];
    entry->func = isr;
    entry->param = parameter;

    NVIC_SetVector(interrupt, (uint32_t)irq_global);

    return FWK_SUCCESS;
}

static int set_isr_nmi(void (*isr)(void))
{
    NVIC_SetVector(NonMaskableInt_IRQn, (uint32_t)isr);

    return FWK_SUCCESS;
}

static int set_isr_nmi_param(void (*isr)(uintptr_t param), uintptr_t parameter)
{
    struct callback *entry;

    entry = &callback[NVIC_USER_IRQ_OFFSET + NonMaskableInt_IRQn - 1];
    entry->func = isr;
    entry->param = parameter;

    NVIC_SetVector(NonMaskableInt_IRQn, (uint32_t)irq_global);

    return FWK_SUCCESS;
}

static int set_isr_fault(void (*isr)(void))
{
    NVIC_SetVector(HardFault_IRQn, (uint32_t)isr);
    NVIC_SetVector(MemoryManagement_IRQn, (uint32_t)isr);
    NVIC_SetVector(BusFault_IRQn, (uint32_t)isr);
    NVIC_SetVector(UsageFault_IRQn, (uint32_t)isr);

    return FWK_SUCCESS;
}

static int get_current(unsigned int *interrupt)
{
    *interrupt = __get_IPSR();

    /* Not an interrupt */
    if (*interrupt == 0)
        return FWK_E_STATE;

    if (*interrupt == (NVIC_USER_IRQ_OFFSET + NonMaskableInt_IRQn))
        *interrupt = FWK_INTERRUPT_NMI;
    else if (*interrupt < NVIC_USER_IRQ_OFFSET)
        *interrupt = FWK_INTERRUPT_EXCEPTION;
    else
        *interrupt -= NVIC_USER_IRQ_OFFSET;

    return FWK_SUCCESS;
}

static const struct fwk_arch_interrupt_driver arch_nvic_driver = {
    .global_enable = global_enable,
    .global_disable = global_disable,
    .is_enabled = is_enabled,
    .enable = enable,
    .disable = disable,
    .is_pending = is_pending,
    .set_pending = set_pending,
    .clear_pending = clear_pending,
    .set_isr_irq = set_isr_irq,
    .set_isr_irq_param = set_isr_irq_param,
    .set_isr_nmi = set_isr_nmi,
    .set_isr_nmi_param = set_isr_nmi_param,
    .set_isr_fault = set_isr_fault,
    .get_current = get_current,
};

static void irq_invalid(void)
{
    disable(__get_IPSR());
}

int arch_nvic_init(const struct fwk_arch_interrupt_driver **driver)
{
    uint32_t ictr_intlinesnum;
    uint32_t align_entries;
    uint32_t align_word;

    uint32_t *vector;
    IRQn_Type irq;

    if (driver == NULL)
        return FWK_E_PARAM;

    /* Find the number of interrupt lines implemented in hardware */
    ictr_intlinesnum = SCnSCB->ICTR & SCnSCB_ICTR_INTLINESNUM_Msk;
    irq_count = (ictr_intlinesnum + 1) * 32;
    isr_count = NVIC_USER_IRQ_OFFSET + irq_count;

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

    /* Allocate and wipe the new vector table */
    vector = fwk_mm_calloc_aligned(align_word, isr_count, sizeof(vector[0]));

    /* Copy the processor exception table over to the new vector table */
    memcpy(
        vector,
        (const void *)SCB->VTOR,
        NVIC_USER_IRQ_OFFSET * sizeof(vector[0]));

    __DMB();

    __disable_irq();

    /* Switch to the new vector table */
    SCB->VTOR = (uint32_t)vector;

    /* Initialize IRQs */
    for (irq = 0; irq < (IRQn_Type)irq_count; irq++) {
        /* Ensure IRQs are disabled during boot sequence */
        NVIC_DisableIRQ(irq);
        NVIC_ClearPendingIRQ(irq);

        /* Initialize all IRQ entries to point to the irq_invalid() handler */
        NVIC_SetVector(irq, (uint32_t)irq_invalid);
    }

    __enable_irq();

    /* Enable the Usage, Bus and Memory faults which are disabled by default */
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk |
        SCB_SHCSR_USGFAULTENA_Msk;

    *driver = &arch_nvic_driver;

    return FWK_SUCCESS;
}
