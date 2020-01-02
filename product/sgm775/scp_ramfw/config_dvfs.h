/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_DVFS_H
#define CONFIG_DVFS_H

enum dvfs_element_idx {
    DVFS_ELEMENT_IDX_LITTLE,
    DVFS_ELEMENT_IDX_BIG,
    DVFS_ELEMENT_IDX_GPU,
    DVFS_ELEMENT_IDX_COUNT
};

#endif /* CONFIG_DVFS_H */
