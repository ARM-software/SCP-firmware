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

extern int fwk_mm_init(uintptr_t start, size_t size);
extern int fwk_interrupt_init(const struct fwk_arch_interrupt_driver *driver);

static int mm_init(int (*mm_init_handler)(struct fwk_arch_mm_data *data))
{
    int status;
    struct fwk_arch_mm_data data;

    /*
     * Retrieve a description of the memory area used for dynamic memory
     * allocation from the architecture layer.
     */
    status = mm_init_handler(&data);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Initialize the memory management component */
    status = fwk_mm_init(data.start, data.size);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int interrupt_init(
    int (*interrupt_init_handler)(struct fwk_arch_interrupt_driver **driver))
{
    int status;
    struct fwk_arch_interrupt_driver *driver;

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

    fwk_log_init();

    if (driver == NULL)
        return FWK_E_PARAM;

    if (driver->mm == NULL)
        return FWK_E_PARAM;

    /* Initialize memory management */
    status = mm_init(driver->mm);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Initialize interrupt management */
    status = interrupt_init(driver->interrupt);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Initialize modules */
    status = __fwk_module_init();
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}
