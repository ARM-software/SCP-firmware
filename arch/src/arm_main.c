/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <fwk_arch.h>
#include <fwk_errno.h>
#include <cmsis_compiler.h>

extern int arm_nvic_init(struct fwk_arch_interrupt_driver **driver);
extern int arm_mm_init(struct fwk_arch_mm_data *data);

/* Platform-dependent backend for the _Exit() function */
void _exit(int status)
{
    while (true)
        __WFI();
}

static struct fwk_arch_init_driver arch_init_driver = {
    .mm = arm_mm_init,
    .interrupt = arm_nvic_init,
};

int main(void)
{
    return fwk_arch_init(&arch_init_driver);
}
