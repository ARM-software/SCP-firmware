/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and TRANSPORT module configurations.
 */

#ifndef MORELLO_SCP_SCMI_H
#define MORELLO_SCP_SCMI_H

/* SCMI agent identifiers */
enum scp_morello_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_ID_OSPM = 1,
    SCP_SCMI_AGENT_ID_PSCI,
    SCP_SCMI_AGENT_ID_MCP,
    SCP_SCMI_AGENT_ID_COUNT
};

/* SCMI service indexes */
enum scp_morello_scmi_service_idx {
    SCP_MORELLO_SCMI_SERVICE_IDX_PSCI,
    SCP_MORELLO_SCMI_SERVICE_IDX_OSPM,
    SCP_MORELLO_SCMI_SERVICE_IDX_MCP,
    SCP_MORELLO_SCMI_SERVICE_IDX_COUNT,
};

#endif /* MORELLO_SCP_SCMI_H */
