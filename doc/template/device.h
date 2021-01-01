/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     < ... >.
 */

#ifndef <DEVICE>_H
#define <DEVICE>_H

#include <fwk_macros.h>

#include <stdint.h>

struct device_reg {
    /* Readable and writable register */
    FWK_RW uint32_t <REGISTER NAME>;
           uint32_t RESERVED1;

    /* Write-only register */
    FWK_W  uint32_t <REGISTER NAME>;

    /* Read-only register */
    FWK_R  uint32_t <REGISTER NAME>;
           uint32_t RESERVED2[0x40];
};

/* Register bit definitions */
#define <DEVICE>_<REGISTER NAME>_<FIELD> UINT32_C(0x00000001)
#define <DEVICE>_<REGISTER NAME>_<FIELD> UINT32_C(0x00000002)

#endif /* <DEVICE>_H */
