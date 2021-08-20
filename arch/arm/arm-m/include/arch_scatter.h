/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Common linker script configuration options.
 *
 * There are three supported memory layouts for the ARM-M architectures:
 *
 * Layout 1 - Single region:
 *      This layout uses a single read/write/execute memory region for all data.
 *      This is traditionally used by firmware running from a general-purpose
 *      RAM region. In this configuration MEM0 represents the RAM region, and
 *      MEM1 is unused.
 *
 * Layout 2 - Dual region with relocation:
 *      This layout uses a read/execute memory region for read-only and
 *      executable data, and a write memory region for writable data. This is
 *      traditionally used by firmware running from a ROM region. In this
 *      configuration MEM0 represents the ROM region and MEM1 represents the RAM
 *      region.
 *
 * Layout 3 - Dual region without relocation:
 *      This layout uses an execute memory region for executable data, and a
 *      read/write memory region for writable data. This is traditionally used
 *      by firmware running from a RAM region attached to the instruction bus.
 *      In this configuration MEM0 represents the RAM region attached to the
 *      instruction bus and MEM1 represents the RAM region attached to the data
 *      bus.
 */

#ifndef ARCH_SCATTER_H
#define ARCH_SCATTER_H

#define ARCH_MEM_MODE_SINGLE_REGION             0
#define ARCH_MEM_MODE_DUAL_REGION_RELOCATION    1
#define ARCH_MEM_MODE_DUAL_REGION_NO_RELOCATION 2

#include <fmw_memory.h>

#ifndef FMW_MEM_MODE
#    error "FMW_MEM_MODE has not been configured"
#endif

#if (FMW_MEM_MODE != ARCH_MEM_MODE_SINGLE_REGION) && \
    (FMW_MEM_MODE != ARCH_MEM_MODE_DUAL_REGION_RELOCATION) && \
    (FMW_MEM_MODE != ARCH_MEM_MODE_DUAL_REGION_NO_RELOCATION)
#    error "FMW_MEM_MODE has been configured improperly"
#endif

#ifndef FMW_MEM0_BASE
#    error "FMW_MEM0_BASE has not been configured"
#endif

#ifndef FMW_MEM0_SIZE
#    error "FMW_MEM0_SIZE has not been configured"
#endif

#define ARCH_MEM0_LIMIT (FMW_MEM0_BASE + FMW_MEM0_SIZE)

#if FMW_MEM_MODE != ARCH_MEM_MODE_SINGLE_REGION
#    ifndef FMW_MEM1_BASE
#        error "FMW_MEM1_BASE has not been configured"
#    endif

#    ifndef FMW_MEM1_SIZE
#        error "FMW_MEM1_SIZE has not been configured"
#    endif

#    define ARCH_MEM1_LIMIT (FMW_MEM1_BASE + FMW_MEM1_SIZE)
#endif

#endif /* ARCH_SCATTER_H */
