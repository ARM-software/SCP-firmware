/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for DVFS module configuration.
 */

#ifndef TC2_DVFS_H
#define TC2_DVFS_H

enum dvfs_element_idx {
    DVFS_ELEMENT_IDX_CORTEX_A520,
    DVFS_ELEMENT_IDX_CORTEX_A720,
    DVFS_ELEMENT_IDX_CORTEX_X4,
    DVFS_ELEMENT_IDX_GPU,
    DVFS_ELEMENT_IDX_COUNT
};

#endif /* TC2_DVFS_H */
