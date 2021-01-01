/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <arch_interrupt.h>

#include <stdio.h>
#include <stdlib.h>

/*
 * Catches early failures in the initialization.
 */
static noreturn void panic(void)
{
    printf("Panic!\n");
    exit(1);
}

static const struct fwk_arch_init_driver arch_init_driver = {
    .interrupt = arch_interrupt_init,
};

int main(void)
{
    int status;

    status = fwk_arch_init(&arch_init_driver);
    if (status != FWK_SUCCESS)
        panic();
}
