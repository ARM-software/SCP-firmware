/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_PIK_H
#define CPU_PIK_H

#include "scp_css_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

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
 * \brief CPU PIK register definitions
 */
struct pik_cpu_reg {
    FWK_RW uint32_t CLUSTER_CONFIG;
    uint8_t RESERVED0[0x100 - 0x4];
    struct static_config_reg STATIC_CONFIG[10];
    uint8_t RESERVED1[0x900 - 0x1a0];
    struct coreclk_reg CORECLK[10];
};

#define CLUSTER_PIK_PTR(IDX) ((struct pik_cpu_reg *)SCP_PIK_CLUSTER_BASE(IDX))

#endif /* CPU_PIK_H */
