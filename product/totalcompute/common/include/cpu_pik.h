/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_PIK_H
#define CPU_PIK_H

#include "scp_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

#define MAX_PIK_SUPPORTED_CPUS 10

/*!
 * \brief PE Static Configuration register definitions
 */
struct static_config_reg {
    FWK_RW uint32_t STATIC_CONFIG;
    FWK_RW uint32_t RVBARADDR_LW;
    FWK_RW uint32_t RVBARADDR_UP;
    uint32_t RESERVED;
};

/*!
 * \brief AP cores clock control register definitions
 */
struct coreclk_reg {
    FWK_RW uint32_t DIV;
    FWK_RW uint32_t CTRL;
    FWK_RW uint32_t MOD;
    uint32_t RESERVED;
};
/*!
 * \brief CPU (v9) PIK register definitions
 */
struct pik_cpu_reg {
    FWK_RW uint32_t CLUSTER_CONFIG;
    uint8_t RESERVED0[0x100 - 0x4];
    struct static_config_reg STATIC_CONFIG[MAX_PIK_SUPPORTED_CPUS];
    uint8_t RESERVED1[0x900 - 0x1a0];
    struct coreclk_reg CORECLK[MAX_PIK_SUPPORTED_CPUS];
};

#define CLUSTER_PIK_PTR ((struct pik_cpu_reg *)SCP_PIK_CLUSTER_BASE)

#endif /* CPU_PIK_H */
