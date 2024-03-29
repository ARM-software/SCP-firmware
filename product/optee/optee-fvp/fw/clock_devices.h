/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
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
    CLOCK_DEV_IDX_VPU,
    CLOCK_DEV_IDX_DPU,
    CLOCK_DEV_IDX_PIXEL_0,
    CLOCK_DEV_IDX_PIXEL_1,
    CLOCK_DEV_IDX_MOCK_0,
    CLOCK_DEV_IDX_MOCK_1,
    CLOCK_DEV_IDX_MOCK_2,
    CLOCK_DEV_IDX_MOCK_3,
    CLOCK_DEV_IDX_COUNT
};

#endif /* CLOCK_DEVICES_H */
