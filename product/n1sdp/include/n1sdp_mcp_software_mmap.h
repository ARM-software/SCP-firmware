/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Software defined memory map shared between SCP and MCP cores.
 */

#ifndef N1SDP_MCP_SOFTWARE_MMAP_H
#define N1SDP_MCP_SOFTWARE_MMAP_H

#include "n1sdp_mcp_mmap.h"

/* SCMI payload size */
#define MCP_SCMI_PAYLOAD_SIZE      (128)

/* SCMI non-secure payload areas */
#define SCMI_PAYLOAD_SCP_TO_MCP_NS (MCP_SCP_SHARED_NONSECURE_RAM)
#define SCMI_PAYLOAD_MCP_TO_SCP_NS (MCP_SCP_SHARED_NONSECURE_RAM + \
                                                MCP_SCMI_PAYLOAD_SIZE)

/* SCMI secure payload areas */
#define SCMI_PAYLOAD_SCP_TO_MCP_S  (MCP_SCP_SHARED_SECURE_RAM)
#define SCMI_PAYLOAD_MCP_TO_SCP_S  (MCP_SCP_SHARED_SECURE_RAM + \
                                                MCP_SCMI_PAYLOAD_SIZE)

#endif /* N1SDP_MCP_SOFTWARE_MMAP_H */
