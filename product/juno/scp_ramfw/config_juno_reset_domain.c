/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <juno_scc.h>
#include <mod_reset_domain.h>
#include <mod_juno_reset_domain.h>
#include <system_mmap.h>

static struct fwk_element juno_reset_element_table[] = {
    [JUNO_RESET_DOMAIN_IDX_UART] = {
        .name = "JUNO_UART",
        .data = &((struct mod_juno_reset_uart_config){
            .reset_reg = &(SCC->VSYS_MANUAL_RESET),
            .reset_mask = (0x1 << 8),
        }),
    },
    [JUNO_RESET_DOMAIN_IDX_COUNT] = { 0 }, /* Termination description */
};

static const struct fwk_element *get_juno_reset_domain_elem_table(
                                     fwk_id_t module_id)
{
    return juno_reset_element_table;
}

struct fwk_module_config config_juno_reset_domain = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_juno_reset_domain_elem_table),
};
