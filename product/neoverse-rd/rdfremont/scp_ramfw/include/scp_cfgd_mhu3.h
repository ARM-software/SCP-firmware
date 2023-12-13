/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for MHUv3 module configuration data in SCP firmware.
 */

#ifndef SCP_CFGD_MHU3_H
#define SCP_CFGD_MHU3_H

/* MHUv3 device indices */
enum scp_cfgd_mod_mhu3_device_idx {
    SCP_CFGD_MOD_MHU3_EIDX_SCP_AP_S_CLUS0,
    SCP_CFGD_MOD_MHU3_EIDX_SCP_RSS_S,
    SCP_CFGD_MOD_MHU3_EIDX_COUNT
};

#endif /* SCP_CFGD_MHU3_H */
