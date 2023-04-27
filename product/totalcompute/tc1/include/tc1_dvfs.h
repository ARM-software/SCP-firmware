/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for DVFS module configuration.
 */

#ifndef TC1_DVFS_H
#define TC1_DVFS_H

enum dvfs_element_idx {
    DVFS_ELEMENT_IDX_CORTEX_A510,
    DVFS_ELEMENT_IDX_CORTEX_A715,
    DVFS_ELEMENT_IDX_CORTEX_X3,
    DVFS_ELEMENT_IDX_COUNT
};

#endif /* TC1_DVFS_H */
