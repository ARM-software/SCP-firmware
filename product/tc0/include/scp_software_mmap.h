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

/* SCP ROM and RAM firmware size loaded on main memory */
#define SCP_BOOT_ROM_SIZE (512 * 1024)
#define SCP_RAM_SIZE (512 * 1024)

/* SCP trusted and non-trusted RAM base address */
#define SCP_TRUSTED_RAM_BASE (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x04000000)
#define SCP_NONTRUSTED_RAM_BASE (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x06000000)

/* Secure Shared memory between AP and SCP */
#define SCP_AP_SHARED_SECURE_BASE (SCP_TRUSTED_RAM_BASE)
#define SCP_AP_SHARED_SECURE_SIZE (4 * FWK_KIB)

/* Non-secure Shared memory between AP and SCP */
#define SCP_AP_SHARED_NONSECURE_BASE (SCP_NONTRUSTED_RAM_BASE)
#define SCP_AP_SHARED_NONSECURE_SIZE (4 * FWK_KIB)

/* AP Context Area */
#define SCP_AP_CONTEXT_BASE \
    (SCP_AP_SHARED_SECURE_BASE + SCP_AP_SHARED_SECURE_SIZE - \
     SCP_AP_CONTEXT_SIZE)
#define SCP_AP_CONTEXT_SIZE (64)

/* SDS Memory Region */
#define SCP_SDS_MEM_BASE (SCP_AP_SHARED_SECURE_BASE)
#define SCP_SDS_MEM_SIZE (3520)

/* SCMI Secure Payload Areas */
#define SCP_SCMI_PAYLOAD_SIZE (128)
#define SCP_SCMI_PAYLOAD_S_A2P_BASE (SCP_SDS_MEM_BASE + SCP_SDS_MEM_SIZE)
#define SCP_SCMI_PAYLOAD_S_P2A_BASE \
    (SCP_SCMI_PAYLOAD_S_A2P_BASE + SCP_SCMI_PAYLOAD_SIZE)

/* SCMI Non-Secure Payload Areas */

#define SCP_SCMI_PAYLOAD0_NS_A2P_BASE (SCP_AP_SHARED_NONSECURE_BASE)
#define SCP_SCMI_PAYLOAD0_NS_P2A_BASE \
    (SCP_SCMI_PAYLOAD0_NS_A2P_BASE + SCP_SCMI_PAYLOAD_SIZE)
#define SCP_SCMI_PAYLOAD1_NS_A2P_BASE \
    (SCP_SCMI_PAYLOAD0_NS_P2A_BASE + SCP_SCMI_PAYLOAD_SIZE)
#define SCP_SCMI_PAYLOAD1_NS_P2A_BASE \
    (SCP_SCMI_PAYLOAD1_NS_A2P_BASE + SCP_SCMI_PAYLOAD_SIZE)

#endif /* SCP_SOFTWARE_MMAP_H */
