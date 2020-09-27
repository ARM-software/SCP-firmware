/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <stdbool.h>
#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <arch_gic.h>
#include <mod_rcar_system.h>

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

    rcar_system_code_copy_to_system_ram();

#ifdef BUILD_MODE_DEBUG
    uint32_t cntv_ctl = 0;
    __asm__ volatile ("msr cntv_ctl_el0, %0" :: "r" (cntv_ctl));
#endif /* BUILD_MODE_DEBUG */

    status = fwk_arch_init(&arch_init_driver);
    if (status != FWK_SUCCESS)
        panic();
}
