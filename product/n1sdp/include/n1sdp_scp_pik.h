/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_SCP_PIK_H
#define N1SDP_SCP_PIK_H

#include "n1sdp_pik_cpu.h"
#include "n1sdp_pik_debug.h"
#include "n1sdp_pik_scp.h"
#include "n1sdp_pik_system.h"
#include "n1sdp_scc_reg.h"
#include "n1sdp_scp_mmap.h"

#define PIK_CLUSTER(IDX) ((struct pik_cpu_reg *)SCP_PIK_CLUSTER_BASE(IDX))
#define PIK_SCP          ((struct pik_scp_reg *)SCP_PIK_SCP_BASE)
#define PIK_SYSTEM       ((struct pik_system_reg *)SCP_PIK_SYSTEM_BASE)
#define PIK_DEBUG        ((struct pik_debug_reg *)SCP_PIK_DEBUG_BASE)
#define SCC              ((struct scc_reg *)SCP_SCC_BASE)

#endif /* N1SDP_SCP_PIK_H */
