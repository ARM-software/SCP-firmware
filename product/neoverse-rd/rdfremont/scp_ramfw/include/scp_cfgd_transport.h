/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for transport module configuration data in SCP
 *     firmware.
 */

#ifndef SCP_CFGD_TRANSPORT_H
#define SCP_CFGD_TRANSPORT_H

/* Module 'transport' element indexes */
enum scp_cfgd_mod_transport_element_idx {
    SCP_CFGD_MOD_TRANSPORT_EIDX_PSCI,
    SCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM,
    SCP_CFGD_MOD_TRANSPORT_EIDX_COUNT,
};

#endif /* SCP_CFGD_TRANSPORT_H */
