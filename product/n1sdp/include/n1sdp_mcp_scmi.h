/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_MCP_SCMI_H
#define N1SDP_MCP_SCMI_H

/* SCMI agent identifiers */
enum mcp_n1sdp_scmi_agent_idx {
    /* 0 is reserved for the platform */
    MCP_N1SDP_SCMI_AGENT_IDX_MANAGEMENT = 1,
    MCP_N1SDP_SCMI_AGENT_IDX_COUNT,
};

/* SCMI service indexes */
enum scp_n1sdp_scmi_service_idx {
    MCP_N1SDP_SCMI_SERVICE_IDX_SCP,
    MCP_N1SDP_SCMI_SERVICE_IDX_COUNT,
};

#endif /* N1SDP_MCP_SCMI_H */
