/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM776_PIK_GPU_H
#define SGM776_PIK_GPU_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief GPU PIK register definitions
 */
struct pik_gpu_reg {
            uint8_t  RESERVED0[0x810];
    FWK_RW  uint32_t GPUCLK_CTRL;
    FWK_RW  uint32_t GPUCLK_DIV1;
    FWK_RW  uint32_t GPUCLK_DIV2;
            uint8_t  RESERVED1[0x820 - 0x81C];
    FWK_RW  uint32_t ACLKGPU_CTRL;
            uint8_t  RESERVED2[0xA00 - 0x824];
    FWK_R   uint32_t CLKFORCE_STATUS;
    FWK_W   uint32_t CLKFORCE_SET;
    FWK_W   uint32_t CLKFORCE_CLR;
            uint8_t  RESERVED3[0xFBC - 0xA0C];
    FWK_R   uint32_t CAP;
    FWK_R   uint32_t PCL_CONFIG;
            uint8_t  RESERVED4[0xFD0 - 0xFC4];
    FWK_R   uint32_t PID4;
    FWK_R   uint32_t PID5;
    FWK_R   uint32_t PID6;
    FWK_R   uint32_t PID7;
    FWK_R   uint32_t PID0;
    FWK_R   uint32_t PID1;
    FWK_R   uint32_t PID2;
    FWK_R   uint32_t PID3;
    FWK_R   uint32_t ID0;
    FWK_R   uint32_t ID1;
    FWK_R   uint32_t ID2;
    FWK_R   uint32_t ID3;
};
#endif  /* SGM776_PIK_GPU_H */
