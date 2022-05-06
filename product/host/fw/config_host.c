/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_noreturn.h>
#include <fwk_status.h>
#include <arch_main.h>

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

/* Main loop */

int main(int argc, char *argv[])
{
     int status;

    /* Initialize scmi server */
    status = scmi_arch_init();
    if (status != FWK_SUCCESS)
        panic();

    for (;;) {
        fwk_process_event_queue();
        (void)fwk_log_unbuffer();
        fwk_arch_suspend();
    }
}
