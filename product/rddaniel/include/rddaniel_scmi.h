/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and SMT module configurations.
 */

#ifndef SCP_RDDANIEL_SCMI_H
#define SCP_RDDANIEL_SCMI_H

/* SCMI agent identifiers */
enum scp_rddaniel_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_ID_PSCI = 1,
};

/* SCMI service indexes */
enum scp_rddaniel_scmi_service_idx {
    SCP_RDDANIEL_SCMI_SERVICE_IDX_PSCI,
    SCP_RDDANIEL_SCMI_SERVICE_IDX_COUNT,
};

#endif /* SCP_RDDANIEL_SCMI_H */
