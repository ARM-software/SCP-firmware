/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System counter implementation specific register defnitions.
 */

#ifndef NEOVERSE_RD_SYSCNT_IMPDEF_H
#define NEOVERSE_RD_SYSCNT_IMPDEF_H

/*
 * Offsets of the system counter implementation defined registers found on
 * RD-N2, RD-Fremont platforms and their variants.
 */
#define NEOVERSE_RD_SYSCNT_IMPDEF0_CNTENCR      0xC0
#define NEOVERSE_RD_SYSCNT_IMPDEF0_CNTSAMPVAL_L 0xC8
#define NEOVERSE_RD_SYSCNT_IMPDEF0_CNTSAMPVAL_H 0xCC
#define NEOVERSE_RD_SYSCNT_IMPDEF0_CNTINCR      0xD0

#endif /* NEOVERSE_RD_SYSCNT_IMPDEF_H */
