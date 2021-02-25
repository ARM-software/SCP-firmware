/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_MOCK_PSU_H
#define CONFIG_MOCK_PSU_H

enum config_mock_psu_element_idx {
    CONFIG_MOCK_PSU_ELEMENT_IDX_CPU_GROUP_LITTLE,
    CONFIG_MOCK_PSU_ELEMENT_IDX_CPU_GROUP_BIG,
    CONFIG_MOCK_PSU_ELEMENT_IDX_GPU,
    CONFIG_MOCK_PSU_ELEMENT_IDX_VPU,
    CONFIG_MOCK_PSU_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_MOCK_PSU_H */
