/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_RDN1E1_PIK_H
#define SCP_RDN1E1_PIK_H

#include "rdn1e1_pik_cpu.h"
#include "rdn1e1_pik_scp.h"
#include "rdn1e1_pik_system.h"
#include "scp_rdn1e1_mmap.h"

#define PIK_CLUSTER(IDX) ((struct pik_cpu_reg *) SCP_PIK_CLUSTER_BASE(IDX))
#define PIK_SCP          ((struct pik_scp_reg *) SCP_PIK_SCP_BASE)
#define PIK_SYSTEM       ((struct pik_system_reg *) SCP_PIK_SYSTEM_BASE)

#endif /* SCP_RDN1E1_PIK_H */
