/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP ROM and RAM memory sizes. These definitions are kept isolated without
 *     the UINT32_C() or UL decorators allowing them to be used in the linker
 *     script.
 */

#ifndef SYSTEM_MMAP_SCP_H
#define SYSTEM_MMAP_SCP_H

#include "sgm775_mmap_scp.h"

#define SCP_ROM_SIZE  (64 * 1024)
#define SCP_RAM_SIZE  (128 * 1024)

#endif /* SYSTEM_MMAP_SCP_H */
