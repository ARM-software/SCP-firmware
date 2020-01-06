/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and SMT module configurations.
 */

#ifndef JUNO_SCMI_H
#define JUNO_SCMI_H

enum juno_scmi_agent_idx {
    /* 0 is reserved for the platform */
    JUNO_SCMI_AGENT_IDX_OSPM = 1,
    JUNO_SCMI_AGENT_IDX_PSCI,
    JUNO_SCMI_AGENT_IDX_COUNT,
};

enum juno_scmi_service_idx {
    JUNO_SCMI_SERVICE_IDX_PSCI,
    JUNO_SCMI_SERVICE_IDX_OSPM_0,
    JUNO_SCMI_SERVICE_IDX_OSPM_1,
    JUNO_SCMI_SERVICE_IDX_COUNT,
};

#endif /* JUNO_SCMI_H */
