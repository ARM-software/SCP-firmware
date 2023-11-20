/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scp_css_mmap.h>

#include <mod_sp805.h>

#include <fwk_interrupt.h>
#include <fwk_module.h>

/* SCP uses NMI as Watchdog interrupt line */
#define SCP_WDOG_IRQ FWK_INTERRUPT_NMI

/* Reload value for the WdogLoad register */
#define LOAD_VALUE 0x0fffffff

/*
 * Watch Dog Timer Driver configuration
 */
const struct fwk_module_config config_sp805 = {
    .data =
        &(struct mod_sp805_config){
            .reg_base = SCP_SP805_WDOG_BASE,
            .wdt_load_value = LOAD_VALUE,
            .driver_id = FWK_ID_NONE_INIT,
            .sp805_irq = SCP_WDOG_IRQ,
        }
};
