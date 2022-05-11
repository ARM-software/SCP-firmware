/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
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
static const struct fwk_arch_interrupt_driver *fwk_interrupt_driver;

int fwk_interrupt_init(const struct fwk_arch_interrupt_driver *driver)
{
    /* Validate driver by checking that all function pointers are non-null */
    if (driver == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->global_enable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->global_disable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->is_enabled == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->enable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->disable == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->is_pending == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_pending == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->clear_pending == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_irq == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_irq_param == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_nmi == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_nmi_param == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->set_isr_fault == NULL) {
        return FWK_E_PARAM;
    }
    if (driver->get_current == NULL) {
        return FWK_E_PARAM;
    }

    fwk_interrupt_driver = driver;
    initialized = true;

    return FWK_SUCCESS;
}

int fwk_interrupt_is_enabled(unsigned int interrupt, bool *enabled)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (enabled == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->is_enabled(interrupt, enabled);
}

int fwk_interrupt_enable(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->enable(interrupt);
}

int fwk_interrupt_disable(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->disable(interrupt);
}

int fwk_interrupt_is_pending(unsigned int interrupt, bool *pending)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (pending == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->is_pending(interrupt, pending);
}

int fwk_interrupt_set_pending(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->set_pending(interrupt);
}

int fwk_interrupt_clear_pending(unsigned int interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    return fwk_interrupt_driver->clear_pending(interrupt);
}

int fwk_interrupt_set_isr(unsigned int interrupt, void (*isr)(void))
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (isr == NULL) {
        return FWK_E_PARAM;
    }

    if (interrupt == FWK_INTERRUPT_NMI) {
        return fwk_interrupt_driver->set_isr_nmi(isr);
    } else {
        return fwk_interrupt_driver->set_isr_irq(interrupt, isr);
    }
}

int fwk_interrupt_set_isr_param(unsigned int interrupt,
                                void (*isr)(uintptr_t param),
                                uintptr_t param)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (isr == NULL) {
        return FWK_E_PARAM;
    }

    if (interrupt == FWK_INTERRUPT_NMI) {
        return fwk_interrupt_driver->set_isr_nmi_param(isr, param);
    } else {
        return fwk_interrupt_driver->set_isr_irq_param(interrupt, isr, param);
    }
}

int fwk_interrupt_get_current(unsigned int *interrupt)
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (interrupt == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->get_current(interrupt);
}

bool fwk_is_interrupt_context(void)
{
    if (!initialized) {
        return false;
    }

    return fwk_interrupt_driver->is_interrupt_context();
}

/* This function is only for internal use by the framework */
int fwk_interrupt_set_isr_fault(void (*isr)(void))
{
    if (!initialized) {
        return FWK_E_INIT;
    }

    if (isr == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_interrupt_driver->set_isr_fault(isr);
}
