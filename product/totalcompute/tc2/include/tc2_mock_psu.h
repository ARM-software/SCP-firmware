/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for mock PSU module configuration.
 */

#ifndef TC2_MOCK_PSU_H
#define TC2_MOCK_PSU_H

enum mock_psu_id {
    MOCK_PSU_ELEMENT_IDX_CORTEX_A520,
    MOCK_PSU_ELEMENT_IDX_CORTEX_A720,
    MOCK_PSU_ELEMENT_IDX_CORTEX_X4,
    MOCK_PSU_ELEMENT_IDX_GPU,
    MOCK_PSU_ELEMENT_IDX_COUNT,
};

#endif /* TC2_MOCK_PSU_H */
