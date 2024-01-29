/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RAM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "lcp_css_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_DUAL_REGION_RELOCATION

/*
 * RAM instruction memory
 */
#define FMW_MEM0_SIZE LCP_ITCM_SIZE
#define FMW_MEM0_BASE LCP_ITCM_S_BASE

/*
 * RAM data memory
 */
#define FMW_MEM1_SIZE LCP_DTCM_SIZE
#define FMW_MEM1_BASE LCP_DTCM_S_BASE

#endif /* FMW_MEMORY_H */
