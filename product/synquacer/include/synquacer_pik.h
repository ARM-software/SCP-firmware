/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_PIK_H
#define SYNQUACER_PIK_H

#include "pik_cpu.h"
#include "pik_scp.h"
#include "pik_system.h"
#include "synquacer_irq.h"
#include "synquacer_mmap.h"

#include <ppu_v0.h>

#include <fwk_macros.h>

#include <fmw_cmsis.h>

#include <stdint.h>

#define PIK_SCP_BASE (POWER_PERIPH_BASE + 0x00000)
#define PIK_DEBUG_BASE (POWER_PERIPH_BASE + 0x20000)
#define PIK_SYSTEM_BASE (POWER_PERIPH_BASE + 0x40000)
#define PIK_CLUSTER_BASE (POWER_PERIPH_BASE + 0x60000)

#define PIK_AP_CLUSTER0_BASE (POWER_PERIPH_BASE + 0x60000)
#define PIK_AP_CLUSTER_INTERVAL (0x20000)
#define PIK_AP_CLUSTER_BASE(n) \
    (PIK_AP_CLUSTER0_BASE + (PIK_AP_CLUSTER_INTERVAL * (n)))

#define PPU_SCP_BASE (PIK_SCP_BASE + 0x1000)
#define PPU_SYS0_BASE (PIK_SYSTEM_BASE + 0x1000)
#define PPU_SYS1_BASE (PIK_SYSTEM_BASE + 0x2000)
#define PPU_SYS2_BASE (PIK_SYSTEM_BASE + 0x3000)
#define PPU_SYS3_BASE (PIK_SYSTEM_BASE + 0x4000)
#define PPU_SYS4_BASE (PIK_SYSTEM_BASE + 0x5000)
#define PPU_DEBUG_BASE (PIK_DEBUG_BASE + 0x1000)
#define PPU_AP_CLUSTER_BASE(n) (PIK_AP_CLUSTER_BASE((n)) + 0x1000)

#define PPU_CLUSTER_AP_CORE_INTERVAL (0x1000)
#define PPU_CPU_BASE(cluster, core)            \
    (PPU_AP_CLUSTER_BASE((cluster)) + 0x1000 + \
     (PPU_CLUSTER_AP_CORE_INTERVAL * (core)))

#define PIK_SCP ((pik_scp_reg_t *)PIK_SCP_BASE)
#define PIK_DEBUG ((pik_debug_reg_t *)PIK_DEBUG_BASE)
#define PIK_SYSTEM ((pik_system_reg_t *)PIK_SYSTEM_BASE)
#define PIK_CLUSTER(n) ((pik_cpu_reg_t *)PIK_AP_CLUSTER_BASE(n))

#define PPU_SCP ((struct ppu_v0_reg *)PPU_SCP_BASE)
#define PPU_SYS0 ((struct ppu_v0_reg *)PPU_SYS0_BASE)
#define PPU_SYS1 ((struct ppu_v0_reg *)PPU_SYS1_BASE)
#define PPU_SYS2 ((struct ppu_v0_reg *)PPU_SYS2_BASE)
#define PPU_SYS3 ((struct ppu_v0_reg *)PPU_SYS3_BASE)
#define PPU_SYS4 ((struct ppu_v0_reg *)PPU_SYS4_BASE)
#define PPU_DEBUG ((struct ppu_v0_reg *)PPU_DEBUG_BASE)
#define PPU_CLUSTER(n) ((struct ppu_v0_reg *)PPU_AP_CLUSTER_BASE(n))
#define PPU_CPU(cluster, core) \
    ((struct ppu_v0_reg *)PPU_CPU_BASE(cluster, core))

#endif /* SYNQUACER_PIK_H */
