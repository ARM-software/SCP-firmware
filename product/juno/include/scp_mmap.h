/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_MMAP_H
#define SCP_MMAP_H

#define SCP_ROM_SIZE  (64 * 1024)
#define SCP_RAM_SIZE  (128 * 1024)

#if SCP_ROM_BYPASS
#define SCP_ROM_BASE  0xABE40000
#else
#define SCP_ROM_BASE  0x00000000
#endif

#define SCP_RAM_BASE  0x10000000

#endif /* SCP_MMAP_H */
