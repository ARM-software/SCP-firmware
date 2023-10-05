/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for PSU module configuration.
 */

#ifndef TC2_PSU_H
#define TC2_PSU_H

enum psu_id {
    PSU_ELEMENT_IDX_CORTEX_A520,
    PSU_ELEMENT_IDX_CORTEX_A720,
    PSU_ELEMENT_IDX_CORTEX_X4,
    PSU_ELEMENT_IDX_GPU,
    PSU_ELEMENT_IDX_COUNT,
};

#endif /* TC2_PSU_H */
