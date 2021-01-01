/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SYSTEM_MMAP_SCP_H
#define SCP_SYSTEM_MMAP_SCP_H

#define SCP_ROM_SIZE (512 * 1024)
#define SCP_RAM0_SIZE (128 * 1024)
#define SCP_RAM1_SIZE (64 * 1024)
#define SCP_RAM2_SIZE (128 * 1024)

#define SCP_ROM_BASE (0x08100000)
#define SCP_RAM0_BASE (0x00800000)
#define SCP_RAM1_BASE (0x00900000)
#define SCP_RAM2_BASE (0x1C020000)

#define SCP_RAMFW_ROM_BASE (0x08110000)
#define SCP_RAMFW_IMAGE_SIZE (128 * 1024)

#endif /* SCP_SYSTEM_MMAP_SCP_H */
