/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Software defined memory map shared between SCP and AP cores.
 */

#ifndef N1SDP_SCP_SOFTWARE_MMAP_H
#define N1SDP_SCP_SOFTWARE_MMAP_H

#include "n1sdp_scp_mmap.h"

#include <fwk_macros.h>

/*
 * Trusted shared SRAM region
 */
#define SCP_AP_SHARED_SECURE_BASE           (SCP_TRUSTED_RAM_BASE)
#define SCP_AP_SHARED_SECURE_SIZE           (4 * FWK_KIB)

/*
 * Non-trusted shared SRAM region
 */
#define SCP_AP_SHARED_NONSECURE_BASE        (SCP_NONTRUSTED_RAM_BASE)
#define SCP_AP_SHARED_NONSECURE_SIZE        (4 * FWK_KIB)

/*
 * SDS Memory Region
 */
#define SCP_SDS_MEM_BASE                    (SCP_AP_SHARED_SECURE_BASE)
#define SCP_SDS_MEM_SIZE                    (3520)

/*
 * AP Context Area
 */
#define SCP_AP_CONTEXT_SIZE                 (64)
#define SCP_AP_CONTEXT_BASE                 (SCP_AP_SHARED_SECURE_BASE + \
                                            SCP_AP_SHARED_SECURE_SIZE - \
                                            SCP_AP_CONTEXT_SIZE)

/*
 * SCMI Secure Payload Areas
 */
#define SCP_SCMI_PAYLOAD_SIZE               (128)
#define SCP_SCMI_PAYLOAD_S_A2P_BASE         (SCP_SDS_MEM_BASE + \
                                            SCP_SDS_MEM_SIZE)
#define SCP_SCMI_PAYLOAD_S_P2A_BASE         (SCP_SCMI_PAYLOAD_S_A2P_BASE + \
                                            SCP_SCMI_PAYLOAD_SIZE)

/*
 * SCMI Non-Secure Payload Areas
 */
#define SCP_SCMI_PAYLOAD_NS_A2P_BASE        (SCP_AP_SHARED_NONSECURE_BASE)
#define SCP_SCMI_PAYLOAD_NS_P2A_BASE        (SCP_SCMI_PAYLOAD_NS_A2P_BASE + \
                                            SCP_SCMI_PAYLOAD_SIZE)

#endif /* N1SDP_SCP_SOFTWARE_MMAP_H */
