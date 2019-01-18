/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_CLARK_PIK_H
#define SCP_CLARK_PIK_H

#include <scp_clark_mmap.h>
#include <clark_pik_cpu.h>
#include <clark_pik_scp.h>
#include <clark_pik_system.h>

#define PIK_CLUSTER(IDX) ((struct pik_cpu_reg *) SCP_PIK_CLUSTER_BASE(IDX))
#define PIK_SCP          ((struct pik_scp_reg *) SCP_PIK_SCP_BASE)
#define PIK_SYSTEM       ((struct pik_system_reg *) SCP_PIK_SYSTEM_BASE)

#endif /* SCP_CLARK_PIK_H */
