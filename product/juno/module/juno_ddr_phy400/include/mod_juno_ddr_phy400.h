/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno DDR-PHY400 driver
 */
#ifndef MOD_JUNO_DDR_PHY400_H
#define MOD_JUNO_DDR_PHY400_H

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdint.h>

/*!
 * \ingroup GroupJunoModule
 * \defgroup GroupDDR400 DDR PHY400 Driver
 * \{
 */

/*!
 * \brief Offset for PTM registers subset.
 */
#define OFFSET_DDR_PHY400_PTM_REGS  0x000

/*!
 * \brief DDR PHY400 PTM register definitions.
 */
struct mod_juno_ddr_phy400_ptm_reg {
    /*!
     * \cond
     * @{
     */
    FWK_RW  uint32_t    INTERRUPT_ENABLE;
    FWK_RW  uint32_t    INTERRUPT_STATUS;
    FWK_RW  uint32_t    BL_APB_CTRL;
    FWK_RW  uint32_t    BL_DISABLE_CTRL;
    FWK_RW  uint32_t    PHY_TRNG_CTRL;
    FWK_RW  uint32_t    PHY_STATUS;
    FWK_RW  uint32_t    PHY_CORE_RSTN_CTRL;
    FWK_RW  uint32_t    PHY_INIT_CTRL;
    FWK_RW  uint32_t    PHY_INIT_STATUS;
    FWK_RW  uint32_t    PHY_TRK_CTRL;
    FWK_RW  uint32_t    MDLLCAL_TRK_CTRL1;
    FWK_RW  uint32_t    MDLLCAL_TRK_CTRL2;
    FWK_RW  uint32_t    MDLLCAL_TRK_STATUS;
    FWK_RW  uint32_t    RESCAL_TRK_CTRL1;
    FWK_RW  uint32_t    RESCAL_TRK_CTRL2;
    FWK_RW  uint32_t    RESCAL_TRK_STATUS;
    FWK_RW  uint32_t    DCC_CTRL;
    FWK_RW  uint32_t    CAC_CK_CAL_CTRL;
    FWK_RW  uint32_t    WRLVL_TRK_DC_DRIFT_CTRL;
    FWK_RW  uint32_t    PHYUPD_TYPE_CTRL;
    FWK_RW  uint32_t    PHY_LP_CTRL;
    FWK_RW  uint32_t    PHY_MISC_CTRL;
    FWK_RW  uint32_t    PHY_BIST_CTRL;
    FWK_RW  uint32_t    PHY_AU_CTRL1;
    FWK_RW  uint32_t    PHY_AU_CTRL2;
    FWK_RW  uint32_t    PHY_AU_CTRL3;
    FWK_RW  uint32_t    PHY_AU_CTRL4;
    FWK_RW  uint32_t    PHY_AU_CTRL5;
    FWK_RW  uint32_t    WRLVL_TRK_STATUS1;
    FWK_RW  uint32_t    WRLVL_TRK_STATUS2;
    FWK_RW  uint32_t    PHY_SCRATCH1;
    FWK_RW  uint32_t    PHY_SCRATCH2;
    /*!
     * \endcond
     * @}
     */
};

/*!
 * \brief Offset for C3A registers subset.
 */
#define OFFSET_DDR_PHY400_C3A_REGS  0x200

/*!
 * \brief DDR PHY400 C3A register definitions.
 */
struct mod_juno_ddr_phy400_c3a_reg {
    /*!
     * \cond
     * @{
     */
    FWK_RW  uint32_t    INTERRUPT_ENABLE;
    FWK_RW  uint32_t    INTERRUPT_STATUS;
    FWK_RW  uint32_t    PLL_CTRL;
    FWK_RW  uint32_t    CAC_CK_CTRL1;
    FWK_RW  uint32_t    CAC_CK_CTRL2;
    FWK_RW  uint32_t    CAC_CK_CAL_CTRL;
    FWK_RW  uint32_t    CAC_CK_CAL_STATUS;
    FWK_RW  uint32_t    RESCAL_CTRL;
    FWK_RW  uint32_t    RESCAL_STATUS;
    FWK_RW  uint32_t    MDLLCAL_CTRL;
    FWK_RW  uint32_t    MDLLCAL_STATUS;
    FWK_RW  uint32_t    DLL_DCC_CTRL;
    FWK_RW  uint32_t    DLL_DCC_STATUS;
    FWK_RW  uint32_t    DLL_MISC_CTRL;
    FWK_RW  uint32_t    WRLVL_TRK_RING_OSC_CTRL;
    FWK_RW  uint32_t    WRLVL_TRK_RING_OSC_STATUS;
    FWK_RW  uint32_t    IO_CTRL1;
    FWK_RW  uint32_t    IO_CTRL2;
    FWK_RW  uint32_t    IO_CTRL3;
    /*!
     * \endcond
     * @}
     */
};

/*!
 * \brief Offset for BL0 registers subset.
 */
#define OFFSET_DDR_PHY400_BL0_REGS  0x400

/*!
 * \brief Offset for BL1 registers subset.
 */
#define OFFSET_DDR_PHY400_BL1_REGS  0x600

/*!
 * \brief Offset for BL2 registers subset.
 */
#define OFFSET_DDR_PHY400_BL2_REGS  0x800

/*!
 * \brief Offset for BL3 registers subset.
 */
#define OFFSET_DDR_PHY400_BL3_REGS  0xA00

/*!
 * \brief DDR PHY400 BL register definitions.
 */
struct mod_juno_ddr_phy400_bl_reg {
    /*!
     * \cond
     * @{
     */
    FWK_RW  uint32_t    INTERRUPT_ENABLE;
    FWK_RW  uint32_t    RAW_INTERRUPT1;
    FWK_RW  uint32_t    RAW_INTERRUPT2;
    FWK_RW  uint32_t    WR_RD_CTRL;
    FWK_RW  uint32_t    WRLVL_CTRL;
    FWK_RW  uint32_t    WRLVL_DLL_CTRL1;
    FWK_RW  uint32_t    WRLVL_DLL_CTRL2;
    FWK_RW  uint32_t    WRLVL_DLL_CTRL3;
    FWK_RW  uint32_t    WRLVL_RANK_MAPPING;
    FWK_RW  uint32_t    WRLVL_STATUS1;
    FWK_RW  uint32_t    WRLVL_STATUS2;
    FWK_RW  uint32_t    WRLVL_STATUS3;
    FWK_RW  uint32_t    WRLVL_STATUS4;
    FWK_RW  uint32_t    RDLVL_WREQ_CTRL;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL1;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL2;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL3;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL4;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL5;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL6;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL7;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL8;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL9;
    FWK_RW  uint32_t    RDLVL_DLL_CTRL10;
    FWK_RW  uint32_t    RDLVL_RANK_MAPPING;
    FWK_RW  uint32_t    WREQ_CTRL;
    FWK_RW  uint32_t    WREQ_DLL_CTRL1;
    FWK_RW  uint32_t    WREQ_DLL_CTRL2;
    FWK_RW  uint32_t    WREQ_DLL_CTRL3;
    FWK_RW  uint32_t    WREQ_DLL_CTRL4;
    FWK_RW  uint32_t    WREQ_DLL_CTRL5;
    FWK_RW  uint32_t    WREQ_DLL_CTRL6;
    FWK_RW  uint32_t    WREQ_DLL_CTRL7;
    FWK_RW  uint32_t    WREQ_DLL_CTRL8;
    FWK_RW  uint32_t    WREQ_DLL_CTRL9;
    FWK_RW  uint32_t    WREQ_DLL_CTRL10;
    FWK_RW  uint32_t    WREQ_DLL_CTRL11;
    FWK_RW  uint32_t    WREQ_RANK_MAPPING;
    FWK_RW  uint32_t    READ_DVW1;
    FWK_RW  uint32_t    READ_DVW2;
    FWK_RW  uint32_t    RDLVL_STATUS2;
    FWK_RW  uint32_t    RDLVL_STATUS3;
    FWK_RW  uint32_t    RDLVL_STATUS4;
    FWK_RW  uint32_t    RDLVL_STATUS5;
    FWK_RW  uint32_t    RDLVL_STATUS6;
    FWK_RW  uint32_t    RDLVL_STATUS7;
    FWK_RW  uint32_t    RDLVL_STATUS8;
    FWK_RW  uint32_t    RDLVL_STATUS9;
    FWK_RW  uint32_t    RDLVL_STATUS10;
    FWK_RW  uint32_t    RDLVL_STATUS11;
    FWK_RW  uint32_t    RDLVL_STATUS12;
    FWK_RW  uint32_t    RDLVL_STATUS13;
    FWK_RW  uint32_t    RDLVL_STATUS14;
    FWK_RW  uint32_t    RDLVL_STATUS15;
    FWK_RW  uint32_t    RDLVL_STATUS16;
    FWK_RW  uint32_t    RDLVL_STATUS17;
    FWK_RW  uint32_t    WRITE_DVW1;
    FWK_RW  uint32_t    WRITE_DVW2;
    FWK_RW  uint32_t    WREQ_STATUS2;
    FWK_RW  uint32_t    WREQ_STATUS3;
    FWK_RW  uint32_t    WREQ_STATUS4;
    FWK_RW  uint32_t    WREQ_STATUS5;
    FWK_RW  uint32_t    WREQ_STATUS6;
    FWK_RW  uint32_t    WREQ_STATUS7;
    FWK_RW  uint32_t    WREQ_STATUS8;
    FWK_RW  uint32_t    WREQ_STATUS9;
    FWK_RW  uint32_t    WREQ_STATUS10;
    FWK_RW  uint32_t    WREQ_STATUS11;
    FWK_RW  uint32_t    WREQ_STATUS12;
    FWK_RW  uint32_t    WREQ_STATUS13;
    FWK_RW  uint32_t    WREQ_STATUS14;
    FWK_RW  uint32_t    WREQ_STATUS15;
    FWK_RW  uint32_t    WREQ_STATUS16;
    FWK_RW  uint32_t    WREQ_STATUS17;
    FWK_RW  uint32_t    WREQ_STATUS18;
    FWK_RW  uint32_t    WREQ_STATUS19;
    FWK_RW  uint32_t    SQUELCH_CTRL;
    FWK_RW  uint32_t    SQUELCH_DIGITAL_DELAY_CTRL;
    FWK_RW  uint32_t    SQUELCH_DLL_CTRL1;
    FWK_RW  uint32_t    SQUELCH_DLL_CTRL2;
    FWK_RW  uint32_t    RDLVL_GATE_RANK_MAPPING;
    FWK_RW  uint32_t    SQUELCH_DIGITAL_DELAY_STATUS;
    FWK_RW  uint32_t    SQUELCH_DLL_STATUS1;
    FWK_RW  uint32_t    SQUELCH_DLL_STATUS2;
    FWK_RW  uint32_t    DLL_MISC_CTRL;
    FWK_RW  uint32_t    IO_CTRL;
    FWK_RW  uint32_t    WRDLL_DCC_CTRL1;
    FWK_RW  uint32_t    WRDLL_DCC_CTRL2;
    FWK_RW  uint32_t    WRDLL_DCC_STATUS;
    /*!
     * \endcond
     * @}
     */
};

/*!
 * \brief Module configuration.
 */
struct mod_juno_ddr_phy400_config {
    /*!
     * \brief Identifier of the timer.
     *
     * \details Used for time-out when configuring the peripheral.
     */
    fwk_id_t timer_id;
};

/*!
 * \brief Element configuration.
 */
struct mod_juno_ddr_phy400_element_config {
    /*! Base address of a device configuration register, PTM subset */
    uintptr_t ddr_phy_ptm;

    /*! Base address of a device configuration register, C3A subset */
    uintptr_t ddr_phy_c3a;

    /*! Base address of a device configuration register, BL0 subset */
    uintptr_t ddr_phy_bl0;

    /*! Base address of a device configuration register, BL1 subset */
    uintptr_t ddr_phy_bl1;

    /*! Base address of a device configuration register, BL2 subset */
    uintptr_t ddr_phy_bl2;

    /*! Base address of a device configuration register, BL3 subset */
    uintptr_t ddr_phy_bl3;
};

/*!
 * @}
 */

#endif /* MOD_JUNO_DDR_PHY400_H */
