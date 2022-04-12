/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mod_hsspi.h"
#include "synquacer_mmap.h"

#include <fwk_module.h>

#include <stddef.h>

/* Configuration of the HSSPI module. */
static struct fwk_element hsspi_dev_table[] = {
    {
        .name = "HSSPI0",
        .sub_element_count = 1,
        .data = &((struct mod_hsspi_dev_config){
            .reg_base = HSSPI_REG_BASE,
            .memory_base = HSSPI_MEM_BASE,
            .clock_div = 4,
            .clock_sel = HSSPI_CLOCK_SELECT_HSEL,
            .deselect_time = HSSPI_DESELECT_TIME_16,
            .memory_bank_size = HSSPI_MEMORY_BANK_SIZE_256M,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct mod_hsspi_config hsspi_config = {
    .bootctl_base = CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP,
};

static const struct fwk_element *hsspi_get_element_table(fwk_id_t module_id)
{
    return hsspi_dev_table;
}

const struct fwk_module_config config_hsspi = {
    .data = (const struct mod_hsspi_config *)&hsspi_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(hsspi_get_element_table),

};
