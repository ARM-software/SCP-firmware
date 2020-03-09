/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno-specific NIC-400 Network Interconnect register definitions.
 */

#ifndef JUNO_NIC400_H
#define JUNO_NIC400_H

#include "juno_mmap.h"

#include <fwk_macros.h>

struct nic400_reg {
    FWK_RW  uint32_t  REMAP;
};

#define NIC400 ((struct nic400_reg *) NIC400_BASE)

#define NIC400_REMAP_TLX_EN UINT32_C(0x00000080)

#endif /* JUNO_NIC400_H */
