/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdnoreturn.h>
#include <fwk_arch.h>
#include <fwk_errno.h>
#include <cmsis_compiler.h>

extern int arm_nvic_init(struct fwk_arch_interrupt_driver **driver);
extern int arm_mm_init(struct fwk_arch_mm_data *data);

/*
 * Error handler for failures that occur during early initialization.
 */
static noreturn void arm_panic(void)
{
    while (true)
        __WFI();
}

static struct fwk_arch_init_driver arch_init_driver = {
    .mm = arm_mm_init,
    .interrupt = arm_nvic_init,
};

void arm_main(void)
{
    int status;

    status = fwk_arch_init(&arch_init_driver);
    if (status != FWK_SUCCESS)
        arm_panic();
}
