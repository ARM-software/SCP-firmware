/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM776_PIK_SCP_H
#define SGM776_PIK_SCP_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief SCP PIK register definitions
 */
struct pik_scp_reg {
            uint8_t  RESERVED0[0x10];
    FWK_RW  uint32_t RESET_SYNDROME;
    FWK_RW  uint32_t WIC_CTRL;
    FWK_R   uint32_t WIC_STATUS;
            uint8_t  RESERVED1[0xA00 - 0x1C];
    FWK_R   uint32_t CLKFORCE_STATUS;
    FWK_RW  uint32_t CLKFORCE_SET;
    FWK_RW  uint32_t CLKFORCE_CLR;
            uint32_t RESERVED2;
    FWK_R   uint32_t PLL_STATUS0;
    FWK_R   uint32_t PLL_STATUS1;
            uint8_t  RESERVED3[0xFC0 - 0xA18];
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

#define PLL_STATUS0_REFCLK              UINT32_C(0x00000001)
#define PLL_STATUS0_GPUPLLLOCK          UINT32_C(0x00000008)
#define PLL_STATUS0_VIDEOPLLLOCK        UINT32_C(0x00000010)
#define PLL_STATUS0_SYSPLLLOCK          UINT32_C(0x00000020)
#define PLL_STATUS0_DISPLAYPLLLOCK      UINT32_C(0x00000040)

#define PLL_STATUS1_CPUPLLLOCK(CORE, PLL)  \
                                      ((uint32_t)((1 << (PLL)) << ((CORE) * 8)))

#define RESET_SYNDROME_PORESET          UINT32_C(0x01)
#define RESET_SYNDROME_WDOGRESET_SCP    UINT32_C(0x02)
#define RESET_SYNDROME_WDOGRESET_SYS    UINT32_C(0x04)
#define RESET_SYNDROME_SYSRESETREQ      UINT32_C(0x08)
#define RESET_SYNDROME_SCPM3LOCKUP      UINT32_C(0x10)

#endif  /* SGM776_PIK_SCP_H */
