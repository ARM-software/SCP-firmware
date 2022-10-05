/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and TRANSPORT module configurations.
 */

#ifndef SCP_SGI575_SCMI_H
#define SCP_SGI575_SCMI_H

/* SCMI agent identifiers */
enum scp_sgi575_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_ID_OSPM = 1,
    SCP_SCMI_AGENT_ID_PSCI,
    SCP_SCMI_AGENT_ID_COUNT,
};

/* SCMI service indexes */
enum scp_sgi575_scmi_service_idx {
    SCP_SGI575_SCMI_SERVICE_IDX_PSCI,
    SCP_SGI575_SCMI_SERVICE_IDX_OSPM,
    SCP_SGI575_SCMI_SERVICE_IDX_COUNT,
};

#endif /* SCP_SGI575_SCMI_H */
