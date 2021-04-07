/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for DVFS module configuration.
 */

#ifndef SCP_TC0_DVFS_H
#define SCP_TC0_DVFS_H

enum dvfs_element_idx {
    DVFS_ELEMENT_IDX_KLEIN,
    DVFS_ELEMENT_IDX_MATTERHORN,
    DVFS_ELEMENT_IDX_MATTERHORN_ELP_ARM,
    DVFS_ELEMENT_IDX_COUNT
};

#endif /* SCP_TC0_DVFS_H */
