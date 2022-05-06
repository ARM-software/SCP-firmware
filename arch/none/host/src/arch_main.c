/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_status.h>
#include <fwk_log.h>

#include <arch_interrupt.h>

static const struct fwk_arch_init_driver scmi_init_driver = {
    .interrupt = arch_interrupt_init,
};

/* SCMI server init/deinit wrapper */
int scmi_arch_init(void)
{
    int status;

    status = fwk_arch_init(&scmi_init_driver);

    fwk_log_flush();

    return status;
}

int scmi_arch_deinit(void)
{
    return fwk_arch_deinit();
}
