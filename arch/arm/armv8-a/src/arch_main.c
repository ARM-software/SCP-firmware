/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_attributes.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <arch_gic.h>
#include <arch_helpers.h>

#include <stdbool.h>

/*
 * This variable is used to ensure spurious nested calls won't
 * enable interrupts. This is been accessed from inline function defined in
 * arch_helpers.h
 */
unsigned int critical_section_nest_level;

FWK_WEAK int _platform_init(void *params)
{
    return FWK_SUCCESS;
}

/*
 * Error handler for failures that occur during early initialization.
 */
void panic(void)
{
    while (true)
        wfi();
}

static struct fwk_arch_init_driver arch_init_driver = {
    .interrupt = arm_gic_init,
};

void arm_main(void)
{
    int status;

    _platform_init(NULL);

#ifdef BUILD_MODE_DEBUG
    uint32_t cntv_ctl = 0;
    __asm__ volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
#endif /* BUILD_MODE_DEBUG */

    status = fwk_arch_init(&arch_init_driver);
    if (status != FWK_SUCCESS)
        panic();
}
