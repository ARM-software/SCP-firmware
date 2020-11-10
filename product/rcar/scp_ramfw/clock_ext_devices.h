/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_EXT_DEVICES_H
#define CLOCK_EXT_DEVICES_H

/*!
 * \brief External clock device indexes.
 */
enum clock_ext_dev_idx {
    CLOCK_EXT_DEV_IDX_X12_CLK,
    CLOCK_EXT_DEV_IDX_X21_CLK,
    CLOCK_EXT_DEV_IDX_X22_CLK,
    CLOCK_EXT_DEV_IDX_X23_CLK,
    CLOCK_EXT_DEV_IDX_AUDIO_CLKOUT,
    CLOCK_EXT_DEV_IDX_AUDIO_CLK_A,
    CLOCK_EXT_DEV_IDX_AUDIO_CLK_C,
    CLOCK_EXT_DEV_IDX_CAN_CLK,
    CLOCK_EXT_DEV_IDX_PCIE_BUS_CLK,
    CLOCK_EXT_DEV_IDX_SCIF_CLK,
    CLOCK_EXT_DEV_IDX_USB3S0_CLK,
    CLOCK_EXT_DEV_IDX_USB_EXTAL_CLK,
    CLOCK_EXT_DEV_IDX_COUNT
};

#endif /* CLOCK_EXT_DEVICES_H */
