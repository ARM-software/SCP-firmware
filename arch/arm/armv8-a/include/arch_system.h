/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARMV8A_SYSTEM_H
#define ARMV8A_SYSTEM_H

#define R_WARMBOOT (0xAA55AA55)
#define R_SUSPEND (0x55AA55AA)
#define R_RESET (0x5555AAAA)
#define R_OFF (0xAAAA5555)
#define R_CLEAR (0)

#ifdef __ASSEMBLER__
.extern _boot_flag.extern _shutdown_request
#else
extern volatile uint32_t _boot_flag;
extern volatile uint32_t _shutdown_request;

#endif /* __ASSEMBLY__ */

#endif /* ARMV8A_SYSTEM_H */
