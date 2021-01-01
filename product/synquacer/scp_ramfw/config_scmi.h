/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_SCMI_H
#define CONFIG_SCMI_H

/* SCMI agent identifiers */
enum scp_synquacer_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_ID_PSCI = 1,
};

/* SCMI service indexes */
enum scp_synquacer_scmi_service_idx {
    SCP_SYNQUACER_SCMI_SERVICE_IDX_PSCI,
    SCP_SYNQUACER_SCMI_SERVICE_IDX_COUNT,
};

#endif /* CONFIG_SCMI_H */
