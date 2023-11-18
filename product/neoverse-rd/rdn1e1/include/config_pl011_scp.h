/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_SCP_PL011_H
#define CONFIG_SCP_PL011_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

enum config_pl011_element_idx {
    CONFIG_PL011_ELEMENT_IDX_SCP_UART,
    CONFIG_PL011_ELEMENT_IDX_COUNT,
};

#define CONFIG_PL011_ELEMENT_ID_SCP_UART_INIT \
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PL011, CONFIG_PL011_ELEMENT_IDX_SCP_UART)

#define CONFIG_PL011_ELEMENT_ID_SCP_UART \
    FWK_ID_ELEMENT(FWK_MODULE_IDX_PL011, CONFIG_PL011_ELEMENT_IDX_SCP_UART)

#endif /* CONFIG_SCP_PL011_H */
