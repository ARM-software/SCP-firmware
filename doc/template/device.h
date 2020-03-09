/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     < ... >.
 */

#ifndef <DEVICE>_H
#define <DEVICE>_H

#    include <fwk_macros.h>

#    include <stdint.h>

typedef struct {
    /* Readable and writable register */
    FWK_RW uint32_t <REGISTER NAME>;
           uint32_t RESERVED1;

    /* Write-only register */
    FWK_W  uint32_t <REGISTER NAME>;

    /* Read-only register */
    FWK_R  uint32_t <REGISTER NAME>;
           uint32_t RESERVED2[0x40];
} <DEVICE NAME>_reg_t;

/* Register bit definitions */
#define <REGISTER NAME>_<FIELD> UINT32_C(0x00000001)
#define <REGISTER NAME>_<FIELD> UINT32_C(0x00000002)

#endif /* <DEVICE>_H */
