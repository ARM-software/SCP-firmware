/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and TRANSPORT module configurations.
 */

#ifndef N1SDP_SCP_SCMI_H
#define N1SDP_SCP_SCMI_H

/* SCMI agent identifiers */
enum scp_n1sdp_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_ID_OSPM = 1,
    SCP_SCMI_AGENT_ID_PSCI,
    SCP_SCMI_AGENT_ID_MCP,
};

/* SCMI service indexes */
enum scp_n1sdp_scmi_service_idx {
    SCP_N1SDP_SCMI_SERVICE_IDX_PSCI,
    SCP_N1SDP_SCMI_SERVICE_IDX_OSPM,
    SCP_N1SDP_SCMI_SERVICE_IDX_MCP,
    SCP_N1SDP_SCMI_SERVICE_IDX_COUNT,
};

#endif /* N1SDP_SCP_SCMI_H */
