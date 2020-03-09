/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Software defined memory map shared between SCP and AP cores.
 */

#ifndef SCP_SOFTWARE_MMAP_H
#define SCP_SOFTWARE_MMAP_H

#include "scp_rdn1e1_mmap.h"

#include <fwk_macros.h>

#define SCP_NOR_FLASH_BASE                  0x08000000
#define SCP_NOR_BASE                        (SCP_NOR_FLASH_BASE + 0x03D80000)
#define SCP_IMAGE_SIZE                      (256 * FWK_KIB)

#define SCP_AP_SHARED_SECURE_BASE           (SCP_TRUSTED_RAM_BASE)
#define SCP_AP_SHARED_SECURE_SIZE           (4 * FWK_KIB)

/* Non-secure shared memory at the base of Non-trusted SRAM */
#define SCP_AP_SHARED_NONSECURE_BASE        (SCP_NONTRUSTED_RAM_BASE)
#define SCP_AP_SHARED_NONSECURE_SIZE        (4 * FWK_KIB)

/* SDS Memory Region */
#define SCP_SDS_MEM_BASE                    (SCP_AP_SHARED_SECURE_BASE)
#define SCP_SDS_MEM_SIZE                    (3520)

/* AP Context Area */
#define SCP_AP_CONTEXT_BASE                 (SCP_AP_SHARED_SECURE_BASE + \
                                            SCP_AP_SHARED_SECURE_SIZE - \
                                            SCP_AP_CONTEXT_SIZE)
#define SCP_AP_CONTEXT_SIZE                 (64)

/* SCMI Secure Payload Areas */
#define SCP_SCMI_PAYLOAD_SIZE               (128)
#define SCP_SCMI_PAYLOAD_S_A2P_BASE         (SCP_SDS_MEM_BASE + \
                                            SCP_SDS_MEM_SIZE)
#define SCP_SCMI_PAYLOAD_S_P2A_BASE         (SCP_SCMI_PAYLOAD_S_A2P_BASE + \
                                            SCP_SCMI_PAYLOAD_SIZE)

/* SCMI Non-Secure Payload Areas */
#define SCP_SCMI_PAYLOAD_NS_A2P_BASE        (SCP_AP_SHARED_NONSECURE_BASE)
#define SCP_SCMI_PAYLOAD_NS_P2A_BASE        (SCP_SCMI_PAYLOAD_NS_A2P_BASE + \
                                            SCP_SCMI_PAYLOAD_SIZE)

#endif /* SCP_SOFTWARE_MMAP_H */
