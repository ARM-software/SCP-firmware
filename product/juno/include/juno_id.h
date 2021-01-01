/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_ID
#define JUNO_ID

enum juno_idx_platform {
    JUNO_IDX_PLATFORM_RTL = 0x0,
    JUNO_IDX_PLATFORM_RESERVED1 = 0x1,
    JUNO_IDX_PLATFORM_RESERVED2 = 0x2,
    JUNO_IDX_PLATFORM_FVP = 0x3,
    JUNO_IDX_PLATFORM_COUNT,
};

enum juno_idx_revision {
    JUNO_IDX_REVISION_R0,
    JUNO_IDX_REVISION_R1,
    JUNO_IDX_REVISION_R2,
    JUNO_IDX_REVISION_COUNT,
};

enum juno_idx_variant {
    JUNO_IDX_VARIANT_A = 0,
    JUNO_IDX_VARIANT_B,
    JUNO_IDX_VARIANT_COUNT,
};

int juno_id_get_platform(enum juno_idx_platform *platform);
int juno_id_get_revision(enum juno_idx_revision *revision);
int juno_id_get_variant(enum juno_idx_variant *variant);

#endif /* JUNO_ID */
