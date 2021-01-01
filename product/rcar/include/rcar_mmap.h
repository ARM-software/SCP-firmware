/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_MMAP_H
#define RCAR_MMAP_H

#include <rcar_def.h>

#include <stdint.h>

/*
 * Top-level base addresses
 */
#define PERIPHERAL_BASE UINT64_C(0xE6000000)

/*
 * Peripherals
 */
#define CPG_BASE (PERIPHERAL_BASE + 0x00150000)
#define MFIS_BASE (PERIPHERAL_BASE + 0x00260000)

#define MFISAREICR_BASE (MFIS_BASE + 0x0400)

/* Memory */
#define TRUSTED_RAM_BASE UINT64_C(0x040000000)
#define NONTRUSTED_RAM_BASE UINT64_C(0x040002000)

#define SCP_SRAM_BASE (0xE6302000U)
#define SCP_SRAM_SIZE (0x00001000U)
#define SCP_SRAM_STACK_BASE (SCP_SRAM_BASE + SCP_SRAM_SIZE)
#define SCP_SRAM_STACK_SIZE (0x00001000U)

#endif /* RCAR_MMAP_H */
