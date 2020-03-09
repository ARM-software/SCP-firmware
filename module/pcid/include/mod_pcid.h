/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PCID_H
#define MOD_PCID_H

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModulePCID PCID
 *
 * \brief Module used to check the peripheral and component IDs.
 *
 * @{
 */

/*!
 * \brief PID and CID registers.
 */
struct mod_pcid_registers {
    FWK_R uint32_t PID4; /*!< Peripheral ID 4 */
    FWK_R uint32_t PID5; /*!< Peripheral ID 5 */
    FWK_R uint32_t PID6; /*!< Peripheral ID 6 */
    FWK_R uint32_t PID7; /*!< Peripheral ID 7 */
    FWK_R uint32_t PID0; /*!< Peripheral ID 0 */
    FWK_R uint32_t PID1; /*!< Peripheral ID 1 */
    FWK_R uint32_t PID2; /*!< Peripheral ID 2 */
    FWK_R uint32_t PID3; /*!< Peripheral ID 3 */
    FWK_R uint32_t CID0; /*!< Component ID 0 */
    FWK_R uint32_t CID1; /*!< Component ID 1 */
    FWK_R uint32_t CID2; /*!< Component ID 2 */
    FWK_R uint32_t CID3; /*!< Component ID 3 */
};

/*!
 * \brief Check peripheral and component id registers against expected values.
 *
 * \param registers Pointer to set of PCID registers to check.
 * \param expected Pointer to set of PCID Registers that denote the expected
 *     values.
 *
 * \pre \p registers must not be NULL
 * \pre \p expected must not be NULL
 *
 * \retval true All the registers have the expected values.
 * \retval false One or more registers do not have the expected values.
 */
bool mod_pcid_check_registers(const struct mod_pcid_registers *registers,
                              const struct mod_pcid_registers *expected);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_PCID_H */
