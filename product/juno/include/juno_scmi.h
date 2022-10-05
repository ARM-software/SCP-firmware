/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI and TRANSPORT module configurations.
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
    JUNO_SCMI_SERVICE_IDX_PSCI_A2P,
    JUNO_SCMI_SERVICE_IDX_OSPM_A2P_0,
    JUNO_SCMI_SERVICE_IDX_OSPM_A2P_1,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    JUNO_SCMI_SERVICE_IDX_OSPM_P2A,
#endif
    JUNO_SCMI_SERVICE_IDX_COUNT,
};

#endif /* JUNO_SCMI_H */
