/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_SCP_PIK_H
#define MORELLO_SCP_PIK_H

#include "morello_pik_cpu.h"
#include "morello_pik_debug.h"
#include "morello_pik_dpu.h"
#include "morello_pik_gpu.h"
#include "morello_pik_scp.h"
#include "morello_pik_system.h"
#include "morello_scc_reg.h"
#include "morello_scp_mmap.h"

#define PIK_CLUSTER(IDX) ((struct pik_cpu_reg *)SCP_PIK_CLUSTER_BASE(IDX))
#define PIK_SCP ((struct pik_scp_reg *)SCP_PIK_SCP_BASE)
#define PIK_SYSTEM ((struct pik_system_reg *)SCP_PIK_SYSTEM_BASE)
#define PIK_DEBUG ((struct pik_debug_reg *)SCP_PIK_DEBUG_BASE)
#define PIK_GPU ((struct pik_gpu_reg *)SCP_PIK_GPU_BASE)
#define PIK_DPU ((struct pik_dpu_reg *)SCP_PIK_DPU_BASE)
#define SCC ((struct scc_reg *)SCP_SCC_BASE)

#endif /* MORELLO_SCP_PIK_H */
