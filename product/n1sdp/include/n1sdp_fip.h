/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_FIP_H
#define N1SDP_FIP_H

#include <mod_fip.h>

enum mod_n1sdp_fip_toc_entry_type {
    MOD_N1SDP_FIP_TOC_ENTRY_MCP_BL2 = MOD_FIP_TOC_ENTRY_COUNT,
    MOD_N1SDP_FIP_TOC_ENTRY_TFA_BL1,
};

#endif /* N1SDP_FIP_H */
