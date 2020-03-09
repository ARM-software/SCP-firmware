/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>

#include <stdio.h>
#include <stdlib.h>

extern int host_interrupt_init(struct fwk_arch_interrupt_driver **driver);

/*
 * Catches early failures in the initialization.
 */
static noreturn void panic(void)
{
    printf("Panic!\n");
    exit(1);
}

static int mm_init(struct fwk_arch_mm_data *data)
{
    const size_t size = 1024 * 1024; /* 1MB of heap */
    void *mem;

    mem = malloc(size);
    if (mem == NULL)
        return FWK_E_NOMEM;

    data->start = (uintptr_t)mem;
    data->size = size;

    return FWK_SUCCESS;
}

static const struct fwk_arch_init_driver arch_init_driver = {
    .mm = mm_init,
    .interrupt = host_interrupt_init,
};

int main(void)
{
    int status;

    status = fwk_arch_init(&arch_init_driver);
    if (status != FWK_SUCCESS)
        panic();
}
