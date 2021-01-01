/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_MOCK_PSU_H
#define CONFIG_MOCK_PSU_H

/* Mock PSU element indices for Juno */
enum mod_mock_psu_element_idx {
    MOD_MOCK_PSU_ELEMENT_IDX_VSYS,
    MOD_MOCK_PSU_ELEMENT_IDX_VBIG,
    MOD_MOCK_PSU_ELEMENT_IDX_VLITTLE,
    MOD_MOCK_PSU_ELEMENT_IDX_VGPU,

    MOD_MOCK_PSU_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_MOCK_PSU_H */
