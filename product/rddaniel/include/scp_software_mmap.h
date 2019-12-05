/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SOFTWARE_MMAP_H
#define SCP_SOFTWARE_MMAP_H

#include <scp_soc_mmap.h>
#include <fwk_macros.h>

/* SCP RAM firmware base and size on the flash */
#define SCP_RAMFW_IMAGE_FLASH_BASE         (SCP_NOR0_FLASH_BASE + 0x03D80000)
#define SCP_RAMFW_IMAGE_FLASH_SIZE         (256 * FWK_KIB)

#endif /* SCP_SOFTWARE_MMAP_H */
