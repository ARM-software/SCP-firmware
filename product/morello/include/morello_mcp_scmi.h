/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_MCP_SCMI_H
#define MORELLO_MCP_SCMI_H

/* SCMI agent identifiers */
enum mcp_morello_scmi_agent_idx {
    /* 0 is reserved for the platform */
    MCP_MORELLO_SCMI_AGENT_IDX_MANAGEMENT = 1,
    MCP_MORELLO_SCMI_AGENT_IDX_COUNT,
};

/* SCMI service indexes */
enum scp_morello_scmi_service_idx {
    MCP_MORELLO_SCMI_SERVICE_IDX_SCP,
    MCP_MORELLO_SCMI_SERVICE_IDX_COUNT,
};

#endif /* MORELLO_MCP_SCMI_H */
