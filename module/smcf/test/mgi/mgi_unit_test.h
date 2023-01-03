/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCF_HARDWARE_UNIT_TEST_H
#define SMCF_HARDWARE_UNIT_TEST_H

/* MGI_MON_IDX */
#define MGI_MON_IDX_MON0  0U
#define MGI_MON_IDX_MON2  2U
#define MGI_MON_IDX_MON5  5U
#define MGI_MON_IDX_MON7  7U
#define MGI_MON_IDX_MON9  9U
#define MGI_MON_IDX_MON10 10U
#define MGI_MON_IDX_MON15 15U
#define MGI_MON_IDX_MON31 31U
#define MGI_NUM_OF_MON    32U

/* MGI_GRP_ID register */
#define MGI_GRP_ID_GRP_ID 0xAA /* Any random ID */
/* Number of Monitors = 32 */
#define MGI_GRP_ID ((MGI_MON_IDX_MON31 << 16) | MGI_GRP_ID_GRP_ID)

/* MGI_FEAT1 */
#define MGI_FEAT1_NUM_OF_REG  3
#define MGI_FEAT1_NUM_OF_BITS 5

/* Mode test value */
#define MODE_VALUE 0x7

#endif /* SMCF_HARDWARE_UNIT_TEST_H */
