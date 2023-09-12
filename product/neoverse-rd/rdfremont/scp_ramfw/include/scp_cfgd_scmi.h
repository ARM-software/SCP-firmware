/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for SCMI module configuration data in SCP firmware.
 */

#ifndef SCP_CFGD_SCMI_H
#define SCP_CFGD_SCMI_H

/* SCMI agent identifier indexes in the SCMI agent table */
enum scp_scmi_agent_idx {
    /* 0 is reserved for the platform */
    SCP_SCMI_AGENT_IDX_PSCI = 1,
    SCP_SCMI_AGENT_IDX_COUNT,
};

/* Module 'scmi' element indexes (SCMI services supported) */
enum scp_cfgd_mod_scmi_element_idx {
    SCP_CFGD_MOD_SCMI_EIDX_PSCI,
    SCP_CFGD_MOD_SCMI_EIDX_COUNT,
};

#endif /* SCP_CFGD_SCMI_H */
