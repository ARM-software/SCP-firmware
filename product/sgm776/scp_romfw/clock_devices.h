/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_DEVICES_H
#define CLOCK_DEVICES_H

/*!
 * \brief Clock device indexes.
 */
enum clock_dev_idx {
    CLOCK_DEV_IDX_BIG,
    CLOCK_DEV_IDX_LITTLE,
    CLOCK_DEV_IDX_GPU,
    CLOCK_DEV_IDX_SYS_NOCMEMCLK,
    CLOCK_DEV_IDX_SYS_FCMCLK,
    CLOCK_DEV_IDX_SYS_GICCLK,
    CLOCK_DEV_IDX_SYS_PCLKSCP,
    CLOCK_DEV_IDX_SYS_SYSPERCLK,
    CLOCK_DEV_IDX_PLL_SWTCLKTCK,
    CLOCK_DEV_IDX_PLL_SYSTEM,
    CLOCK_DEV_IDX_COUNT
};

#endif /* CLOCK_DEVICES_H */
