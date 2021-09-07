/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SP805 Watchdog Driver
 */

#ifndef MOD_SP805_H
#define MOD_SP805_H

#include <fwk_id.h>

/*!
 * Macros to enable/disable write access to SP805 registers. Writing the value
 * 0x1ACCE551 to WdogLock register enables write access to all other registers.
 * Writing any other value disables write access.
 */
#define ENABLE_WR_ACCESS  0x1ACCE551
#define DISABLE_WR_ACCESS 0xFFFFFFFF

/*!
 * Macros for enabling SP805 module reset output and interrupt event.
 */
#define RESET_EN (1 << 1)
#define INT_EN   (1 << 0)

/*!
 * \brief Represent SP805 WDT module register definitions.
 */
struct sp805_reg {
    /*!
     * Load Register, contains the value from which the counter is to decrement
     */
    FWK_RW uint32_t LOAD;

    /*! Value Register, gives the current value of the decrementing counter */
    FWK_R uint32_t VALUE;

    /*! Control register, enables the software to control the Watchdog module */
    FWK_RW uint32_t CONTROL;

    /*!
     * Interrupt Clear Register, A write of any value to this location clears
     * the Watchdog module interrupt, and reloads the counter from the value in
     * the WdogLoad Register
     */
    FWK_W uint32_t INTCLR;

    /*!
     * Raw Interrupt Status Register, indicates the raw interrupt status from
     * the counter
     */
    FWK_R uint32_t RIS;

    /*!
     * Masked Interrupt Status Register,indicates the masked interrupt status
     * from the counter
     */
    FWK_R uint32_t MIS;

    /*! Reserved region 1*/
    uint8_t RESERVED1[0xC00 - 0x18];

    /*! Lock Register, allows write-access to all other registers */
    FWK_RW uint32_t LOCK;

    /*! Reserved region 2*/
    uint8_t RESERVED2[0xF00 - 0xC04];

    /*!
     * Integration Test Control Register, use to enable integration test mode
     */
    FWK_RW uint32_t ITCR;

    /*!
     * Integration Test Output Set Register, When in integration test mode, the
     * enabled interrupt output and reset output are driven directly from the
     * values in this register
     */
    FWK_W uint32_t ITOP;

    /*! Reserved region 3*/
    uint8_t RESERVED3[0xFE0 - 0xF08];

    /*! Peripheral identification register 0 */
    FWK_R uint32_t PERIPHID0;

    /*! Peripheral identification register 1 */
    FWK_R uint32_t PERIPHID1;

    /*! Peripheral identification register 2 */
    FWK_R uint32_t PERIPHID2;

    /*! Peripheral identification register 3 */
    FWK_R uint32_t PERIPHID3;

    /*! PrimeCell Identification Register 0 */
    FWK_R uint32_t PCELLID0;

    /*! PrimeCell Identification Register 1 */
    FWK_R uint32_t PCELLID1;

    /*! PrimeCell Identification Register 2 */
    FWK_R uint32_t PCELLID2;

    /*! PrimeCell Identification Register 3 */
    FWK_R uint32_t PCELLID3;
};

/*!
 * \brief Configuration data for a SP805 WatchDog device.
 */
struct mod_sp805_config {
    /*! Base address of the device registers */
    const uintptr_t reg_base;

    /*! Watchdog Timer value to be loaded in the timer register */
    const unsigned int wdt_load_value;

    /*!
     * Identifier of the clock that this device depends on. If the device is not
     * dependent on any clock, FWK_ID_NONE_INIT should be assigned for this
     * configuration value.
     */
    fwk_id_t driver_id;

    /*!
     * Watch Dog device IRQ number
     *
     * Please note that sp805 driver is expecting a valid interrupt number in
     * sp805_irq. If FWK_INTERRUPT_NONE is assigned, the SP805 controller is not
     * enabled.
     */
    const unsigned int sp805_irq;
};

#endif /* MOD_SP805_H */
