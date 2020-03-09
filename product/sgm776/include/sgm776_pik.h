/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM776_PIK_H
#define SGM776_PIK_H

#include "sgm776_mmap.h"
#include "sgm776_pik_cpu.h"
#include "sgm776_pik_debug.h"
#include "sgm776_pik_dpu.h"
#include "sgm776_pik_gpu.h"
#include "sgm776_pik_scp.h"
#include "sgm776_pik_system.h"
#include "sgm776_pik_vpu.h"

#define PIK_CLUS0     ((struct pik_cpu_reg *) PIK_CLUS0_BASE)
#define PIK_CLUS1     ((struct pik_cpu_reg *) PIK_CLUS1_BASE)
#define PIK_DEBUG     ((struct pik_debug_reg *) PIK_DEBUG_BASE)
#define PIK_DPU       ((struct pik_dpu_reg *) PIK_DPU_BASE)
#define PIK_GPU       ((struct pik_gpu_reg *) PIK_GPU_BASE)
#define PIK_SCP       ((struct pik_scp_reg *) PIK_SCP_BASE)
#define PIK_SYSTEM    ((struct pik_system_reg *) PIK_SYSTEM_BASE)
#define PIK_VPU       ((struct pik_vpu_reg *) PIK_VPU_BASE)

#endif /* SGM776_PIK_H */
