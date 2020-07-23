/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Framework API for the architecture layer.
 */

#include <internal/fwk_module.h>

#include <fwk_arch.h>
#include <fwk_log.h>
#include <fwk_status.h>

extern int fwk_interrupt_init(const struct fwk_arch_interrupt_driver *driver);

static int fwk_arch_interrupt_init(int (*interrupt_init_handler)(
    const struct fwk_arch_interrupt_driver **driver))
{
    int status;
    const struct fwk_arch_interrupt_driver *driver;

    /*
     * Retrieve a pointer to the interrupt management driver from the
     * architecture layer.
     */
    status = interrupt_init_handler(&driver);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Initialize the interrupt management component */
    status = fwk_interrupt_init(driver);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

int fwk_arch_init(const struct fwk_arch_init_driver *driver)
{
    int status;

    if (driver == NULL)
        return FWK_E_PARAM;

    if (driver->interrupt == NULL)
        return FWK_E_PARAM;

    /* Initialize interrupt management */
    status = fwk_arch_interrupt_init(driver->interrupt);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Initialize modules */
    status = fwk_module_init();
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}
