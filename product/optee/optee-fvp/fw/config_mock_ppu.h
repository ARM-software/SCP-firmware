/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_MOCK_PPU_H
#define CONFIG_MOCK_PPU_H

enum mock_ppu_element_idx {
    MOCK_PPU_ELEMENT_IDX_CPU0,
    MOCK_PPU_ELEMENT_IDX_CPU1,
    MOCK_PPU_ELEMENT_IDX_CPU2,
    MOCK_PPU_ELEMENT_IDX_CPU3,
    MOCK_PPU_ELEMENT_IDX_CLUSTER0,
    MOCK_PPU_ELEMENT_IDX_CLUSTER1,
    MOCK_PPU_ELEMENT_IDX_DBGTOP,
    MOCK_PPU_ELEMENT_IDX_DPU0TOP,
    MOCK_PPU_ELEMENT_IDX_DPU1TOP,
    MOCK_PPU_ELEMENT_IDX_GPUTOP,
    MOCK_PPU_ELEMENT_IDX_VPUTOP,
    MOCK_PPU_ELEMENT_IDX_SYS0,
    MOCK_PPU_ELEMENT_IDX_SYS1,
    MOCK_PPU_ELEMENT_IDX_SYSTEM,
    MOCK_PPU_ELEMENT_IDX_COUNT
};

#endif /* CONFIG_MOCK_PPU_H */
