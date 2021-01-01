/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno PPU element indices.
 */

#ifndef JUNO_PPU_IDX_H
#define JUNO_PPU_IDX_H

enum juno_ppu_idx {
    JUNO_PPU_DEV_IDX_BIG_CPU0,
    JUNO_PPU_DEV_IDX_BIG_CPU1,
    JUNO_PPU_DEV_IDX_BIG_SSTOP,

    JUNO_PPU_DEV_IDX_LITTLE_CPU0,
    JUNO_PPU_DEV_IDX_LITTLE_CPU1,
    JUNO_PPU_DEV_IDX_LITTLE_CPU2,
    JUNO_PPU_DEV_IDX_LITTLE_CPU3,
    JUNO_PPU_DEV_IDX_LITTLE_SSTOP,

    JUNO_PPU_DEV_IDX_GPUTOP,
    JUNO_PPU_DEV_IDX_SYSTOP,
    JUNO_PPU_DEV_IDX_DBGSYS,

    JUNO_PPU_DEV_IDX_COUNT,
};

#endif /* JUNO_PPU_IDX_H */
