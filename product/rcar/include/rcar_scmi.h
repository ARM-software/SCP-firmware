/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_SCMI_H
#define RCAR_SCMI_H

/* SCMI agent identifiers */
enum rcar_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCMI_AGENT_ID_OSPM = 1,
    SCMI_AGENT_ID_PSCI,
    SCMI_AGENT_ID_COUNT,
};

/* SCMI service indexes */
enum rcar_scmi_service_idx {
    RCAR_SCMI_SERVICE_IDX_PSCI,
    RCAR_SCMI_SERVICE_IDX_OSPM_0,
    RCAR_SCMI_SERVICE_IDX_COUNT,
};
#endif /* RCAR_SCMI_H */
