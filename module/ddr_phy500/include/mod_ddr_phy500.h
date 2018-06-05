 /*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DDR-PHY500 driver
 */
#ifndef MOD_DDR_PHY500_H
#define MOD_DDR_PHY500_H

#include <stdint.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <mod_log.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \addtogroup GroupDDR DDR PHY500 Driver
 * @{
 */

/*!
 * \brief DDR PHY500 register definitions.
 */
struct mod_ddr_phy500_reg {
    /*!
     * \cond
     * @{
     */
    FWK_RW  uint32_t      INIT_COMPLETE;
    FWK_RW  uint32_t      MEMORY_WIDTH;
    FWK_RW  uint32_t      READ_DELAY;
    FWK_RW  uint32_t      CAPTURE_MASK;
    FWK_RW  uint32_t      CAS_LATENCY;
    FWK_RW  uint32_t      T_CTRL_DELAY;
    FWK_RW  uint32_t      T_WRLAT;
    FWK_RW  uint32_t      T_RDDATA_EN;
    FWK_RW  uint32_t      T_RDLAT;
    FWK_RW  uint32_t      RESERVED1;
    FWK_R   uint32_t      RESERVED2;
    FWK_R   uint32_t      LP_REQ;
    FWK_RW  uint32_t      LP_ACK;
    FWK_RW  uint32_t      RDLVL_REQ;
    FWK_R   uint32_t      RDLVL_EN;
    FWK_RW  uint32_t      WRLVL_REQ;
    FWK_RW  uint32_t      WRLVL_EN;
    FWK_RW  uint32_t      MSTR_REQ;
    FWK_RW  uint32_t      MSTR_ACK;
    FWK_RW  uint32_t      DFI_WR_PREMBL;
    FWK_R   uint8_t       RESERVED3[0x820 - 0x50];
    FWK_RW  uint32_t      DELAY_SEL;
    FWK_R   uint32_t      RESERVED4;
    FWK_RW  uint32_t      T_CTRL_UPD_MIN;
    /*!
     * \endcond
     * @}
     */
};

/*!
 * \brief Element configuration.
 */
struct mod_ddr_phy500_element_config {
    /*! Base address of a device configuration register. */
    uintptr_t ddr;
};

/*!
 * \brief DDR PHY500 module configuration.
 */
struct mod_ddr_phy500_module_config {
    /*!
    * Pointer to a structure containing default values for a subset of the PHY's
    * configuration registers. These values are common to all PHYs that are
    * represented by elements in the module's element table.
    */
    const struct mod_ddr_phy500_reg *ddr_reg_val;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_DDR_PHY500_H */
