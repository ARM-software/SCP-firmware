/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SOFTWARE_MMAP_H
#define SCP_SOFTWARE_MMAP_H


#include "scp_soc_mmap.h"

#include <fwk_macros.h>

#define SCP_ITC_RAM_SIZE                   (256 * 1024)

/* SCP trusted and non-trusted RAM base address */
#define SCP_TRUSTED_RAM_BASE               (SCP_SYSTEM_ACCESS_PORT1_BASE + \
                                            0x04000000)

/* Secure Shared memory between AP and SCP */
#define SCP_AP_SHARED_SECURE_BASE          (SCP_TRUSTED_RAM_BASE)
#define SCP_AP_SHARED_SECURE_SIZE          (4 * FWK_KIB)

/* AP Context Area */
#define SCP_AP_CONTEXT_BASE                (SCP_AP_SHARED_SECURE_BASE + \
                                           SCP_AP_SHARED_SECURE_SIZE - \
                                           SCP_AP_CONTEXT_SIZE)
#define SCP_AP_CONTEXT_SIZE                (64)

/* SDS Memory Region */
#define SCP_SDS_MEM_BASE                   (SCP_AP_SHARED_SECURE_BASE)
#define SCP_SDS_MEM_SIZE                   (3520)

#endif /* SCP_SOFTWARE_MMAP_H */
