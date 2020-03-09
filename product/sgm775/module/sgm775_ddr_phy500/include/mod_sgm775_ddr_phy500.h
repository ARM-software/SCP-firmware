/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 DDR-PHY500 module
 */
#ifndef MOD_SGM775_DDR_PHY500_H
#define MOD_SGM775_DDR_PHY500_H

#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdint.h>

/*!
 * \addtogroup GroupSGM775Module SGM775 Product Modules
 * @{
 */

/*!
 * \addtogroup GroupSGM775_DDR SGM775 DDR PHY500 Driver
 * @{
 */

/*!
 * \brief DDR PHY500 register definitions.
 */
struct mod_sgm775_ddr_phy500_reg {
    /*!
     * \cond
     * @{
     */
    FWK_RW  uint32_t INIT_COMPLETE;
    FWK_RW  uint32_t MEMORY_WIDTH;
    FWK_RW  uint32_t READ_DELAY;
    FWK_RW  uint32_t CAPTURE_MASK;
    FWK_RW  uint32_t CAS_LATENCY;
    FWK_RW  uint32_t T_CTRL_DELAY;
    FWK_RW  uint32_t T_WRLAT;
    FWK_RW  uint32_t T_RDDATA_EN;
    FWK_RW  uint32_t T_RDLAT;
    FWK_RW  uint32_t DFI_PHYUPD_REQ;
    FWK_R   uint32_t DFI_PHYUPD_ACK;
    FWK_R   uint32_t DFI_LP_REQ;
    FWK_RW  uint32_t DFI_LP_ACK;
    FWK_RW  uint32_t DFI_RDLVL_REQ;
    FWK_R   uint32_t DFI_RDLVL_EN;
    FWK_RW  uint32_t DFI_WRLVL_REQ;
    FWK_R   uint32_t DFI_WRLVL_EN;
    FWK_RW  uint32_t DFI_PHYMSTR_REQ;
    FWK_RW  uint32_t DFI_PHYMSTR_ACK;
    FWK_RW  uint32_t DFI_WR_PREMBL;
            uint8_t  RESERVED[0x820 - 0x50];
    FWK_RW  uint32_t DELAY_SEL;
    FWK_RW  uint32_t REF_EN;
    FWK_RW  uint32_t T_CTRL_UPD_MIN;
    /*!
     * \endcond
     * @}
     */
};

/*!
 * \brief Element configuration.
 */
struct mod_sgm775_ddr_phy500_element_config {
    /*! Base address of a device configuration register. */
    uintptr_t ddr;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SGM775_DDR_PHY500_H */
