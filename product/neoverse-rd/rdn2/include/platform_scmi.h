/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and TRANSPORT module configurations.
 */

#ifndef SCP_PLATFORM_SCMI_H
#define SCP_PLATFORM_SCMI_H

/* SCMI agent identifiers */
enum scp_platform_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_ID_PSCI = 1,
};

/* SCMI service indexes */
enum scp_platform_scmi_service_idx {
    SCP_PLATFORM_SCMI_SERVICE_IDX_PSCI,
    SCP_PLATFORM_SCMI_SERVICE_IDX_COUNT,
};

#endif /* SCP_PLATFORM_SCMI_H */
