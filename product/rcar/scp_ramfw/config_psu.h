/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_PSU_H
#define CONFIG_PSU_H

/* PSU indices for rcar */
enum mod_rcar_psu_element_idx {
    MOD_PSU_ELEMENT_IDX_VLITTLE,
    MOD_PSU_ELEMENT_IDX_VBIG,
    MOD_PSU_ELEMENT_IDX_VGPU,

    /* Number of sensors */
    MOD_PSU_ELEMENT_IDX_COUNT
};

#endif /* CONFIG_PSU_H */
