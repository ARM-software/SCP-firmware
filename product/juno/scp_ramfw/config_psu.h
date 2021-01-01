/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_PSU_H
#define CONFIG_PSU_H

/* PSU indices for Juno */
enum mod_juno_psu_element_idx {
    MOD_PSU_ELEMENT_IDX_VSYS,
    MOD_PSU_ELEMENT_IDX_VBIG,
    MOD_PSU_ELEMENT_IDX_VLITTLE,
    MOD_PSU_ELEMENT_IDX_VGPU,

    /* Number of sensors */
    MOD_PSU_ELEMENT_IDX_COUNT
};

#endif /* CONFIG_PSU_H */
