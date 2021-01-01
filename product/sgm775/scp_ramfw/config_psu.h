/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_PSU_H
#define CONFIG_PSU_H

enum config_psu_element_idx {
    CONFIG_PSU_ELEMENT_IDX_CPU_GROUP_LITTLE,
    CONFIG_PSU_ELEMENT_IDX_CPU_GROUP_BIG,
    CONFIG_PSU_ELEMENT_IDX_GPU,
    CONFIG_PSU_ELEMENT_IDX_VPU,

    CONFIG_PSU_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_PSU_H */
