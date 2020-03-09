/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interrupt management.
 */

#include <internal/fwk_interrupt.h>

#include <fwk_arch.h>
#include <fwk_interrupt.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

static bool initialized;
static const struct fwk_arch_interrupt_driver *driver;
static unsigned int critical_section_nest_level;

int fwk_interrupt_init(const struct fwk_arch_interrupt_driver *_driver)
{
    /* Validate driver by checking that all function pointers are non-null */
    if (_driver == NULL)
        return FWK_E_PARAM;
    if (_driver->global_enable == NULL)
        return FWK_E_PARAM;
    if (_driver->global_disable == NULL)
        return FWK_E_PARAM;
    if (_driver->is_enabled == NULL)
        return FWK_E_PARAM;
    if (_driver->enable == NULL)
        return FWK_E_PARAM;
    if (_driver->disable == NULL)
        return FWK_E_PARAM;
    if (_driver->is_pending == NULL)
        return FWK_E_PARAM;
    if (_driver->set_pending == NULL)
        return FWK_E_PARAM;
    if (_driver->clear_pending == NULL)
        return FWK_E_PARAM;
    if (_driver->set_isr_irq == NULL)
        return FWK_E_PARAM;
    if (_driver->set_isr_irq_param == NULL)
        return FWK_E_PARAM;
    if (_driver->set_isr_nmi == NULL)
        return FWK_E_PARAM;
    if (_driver->set_isr_nmi_param == NULL)
        return FWK_E_PARAM;
    if (_driver->set_isr_fault == NULL)
        return FWK_E_PARAM;
    if (_driver->get_current == NULL)
        return FWK_E_PARAM;

    driver = _driver;
    initialized = true;

    return FWK_SUCCESS;
}

int fwk_interrupt_global_enable(void)
{
    if (!initialized)
        return FWK_E_INIT;

    /* Decrement critical_section_nest_level only if in critical section */
    if (critical_section_nest_level > 0)
        critical_section_nest_level--;

    /* Enable interrupts globally if now outside critical section */
    if (critical_section_nest_level == 0)
        return driver->global_enable();

    return FWK_SUCCESS;
}

int fwk_interrupt_global_disable(void)
{
    if (!initialized)
        return FWK_E_INIT;

    critical_section_nest_level++;

    /* If now in outer-most critical section, disable interrupts globally */
    if (critical_section_nest_level == 1)
        return driver->global_disable();

    return FWK_SUCCESS;
}

int fwk_interrupt_is_enabled(unsigned int interrupt, bool *enabled)
{
    if (!initialized)
        return FWK_E_INIT;

    if (enabled == NULL)
        return FWK_E_PARAM;

    return driver->is_enabled(interrupt, enabled);
}

int fwk_interrupt_enable(unsigned int interrupt)
{
    if (!initialized)
        return FWK_E_INIT;

    return driver->enable(interrupt);
}

int fwk_interrupt_disable(unsigned int interrupt)
{
    if (!initialized)
        return FWK_E_INIT;

    return driver->disable(interrupt);
}

int fwk_interrupt_is_pending(unsigned int interrupt, bool *pending)
{
    if (!initialized)
        return FWK_E_INIT;

    if (pending == NULL)
        return FWK_E_PARAM;

    return driver->is_pending(interrupt, pending);
}

int fwk_interrupt_set_pending(unsigned int interrupt)
{
    if (!initialized)
        return FWK_E_INIT;

    return driver->set_pending(interrupt);
}

int fwk_interrupt_clear_pending(unsigned int interrupt)
{
    if (!initialized)
        return FWK_E_INIT;

    return driver->clear_pending(interrupt);
}

int fwk_interrupt_set_isr(unsigned int interrupt, void (*isr)(void))
{
    if (!initialized)
        return FWK_E_INIT;

    if (isr == NULL)
        return FWK_E_PARAM;

    if (interrupt == FWK_INTERRUPT_NMI)
        return driver->set_isr_nmi(isr);
    else
        return driver->set_isr_irq(interrupt, isr);
}

int fwk_interrupt_set_isr_param(unsigned int interrupt,
                                void (*isr)(uintptr_t param),
                                uintptr_t param)
{
    if (!initialized)
        return FWK_E_INIT;

    if (isr == NULL)
        return FWK_E_PARAM;

    if (interrupt == FWK_INTERRUPT_NMI)
        return driver->set_isr_nmi_param(isr, param);
    else
        return driver->set_isr_irq_param(interrupt, isr, param);
}

int fwk_interrupt_get_current(unsigned int *interrupt)
{
    if (!initialized)
        return FWK_E_INIT;

    if (interrupt == NULL)
        return FWK_E_PARAM;

    return driver->get_current(interrupt);
}

/* This function is only for internal use by the framework */
int fwk_interrupt_set_isr_fault(void (*isr)(void))
{
    if (!initialized)
        return FWK_E_INIT;

    if (isr == NULL)
        return FWK_E_PARAM;

    return driver->set_isr_fault(isr);
}
