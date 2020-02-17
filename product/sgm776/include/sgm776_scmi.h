/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and SMT module configurations.
 */

#ifndef SGM776_SCMI_H
#define SGM776_SCMI_H

/* SCMI agent identifiers */
enum sgm776_scmi_agent_id {
    /* 0 is reserved for the platform */
    SCMI_AGENT_ID_OSPM = 1,
    SCMI_AGENT_ID_PSCI,
    SCMI_AGENT_ID_COUNT
};

/* SCMI service indexes */
enum sgm776_scmi_service_idx {
    SGM776_SCMI_SERVICE_IDX_PSCI,
    SGM776_SCMI_SERVICE_IDX_OSPM_0,
    SGM776_SCMI_SERVICE_IDX_OSPM_1,
    SGM776_SCMI_SERVICE_IDX_COUNT,
};
#endif /* SGM776_SCMI_H */
