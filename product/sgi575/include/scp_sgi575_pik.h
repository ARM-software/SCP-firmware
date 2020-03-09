/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SGI575_PIK_H
#define SCP_SGI575_PIK_H

#include "scp_sgi575_mmap.h"
#include "sgi575_pik_cpu.h"
#include "sgi575_pik_scp.h"
#include "sgi575_pik_system.h"

#define PIK_CLUSTER(IDX) ((struct pik_cpu_reg *) SCP_PIK_CLUSTER_BASE(IDX))
#define PIK_SCP          ((struct pik_scp_reg *) SCP_PIK_SCP_BASE)
#define PIK_SYSTEM       ((struct pik_system_reg *) SCP_PIK_SYSTEM_BASE)

#endif /* SCP_SGI575_PIK_H */
