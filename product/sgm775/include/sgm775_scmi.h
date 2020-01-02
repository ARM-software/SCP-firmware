/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and SMT module configurations.
 */

#ifndef SGM775_SCMI_H
#define SGM775_SCMI_H

/* SCMI agent identifiers */
enum sgm775_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCMI_AGENT_ID_OSPM = 1,
    SCMI_AGENT_ID_PSCI,
    SCMI_AGENT_ID_COUNT,
};

/* SCMI service indexes */
enum sgm775_scmi_service_idx {
    SGM775_SCMI_SERVICE_IDX_PSCI,
    SGM775_SCMI_SERVICE_IDX_OSPM_0,
    SGM775_SCMI_SERVICE_IDX_OSPM_1,
    SGM775_SCMI_SERVICE_IDX_COUNT,
};
#endif /* SGM775_SCMI_H */
