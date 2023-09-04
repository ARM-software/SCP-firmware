/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
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
 * \{
 */

/*!
 * \defgroup GroupModulePCID PCID
 *
 * \brief Module used to check the peripheral and component IDs.
 *
 * \{
 */

/*!
 * Value indicating that a PCID register is valid.
 */
#define PCID_REG_VALID (1u)

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
 * \brief PID/CID register identifier in address order.
 */
enum mod_pcid_register_id {
    MOD_PCID_REGISTER_NONE = 0x000,
    MOD_PCID_REGISTER_PID4 = (PCID_REG_VALID << 0),
    MOD_PCID_REGISTER_PID5 = (PCID_REG_VALID << 1),
    MOD_PCID_REGISTER_PID6 = (PCID_REG_VALID << 2),
    MOD_PCID_REGISTER_PID7 = (PCID_REG_VALID << 3),
    MOD_PCID_REGISTER_PID0 = (PCID_REG_VALID << 4),
    MOD_PCID_REGISTER_PID1 = (PCID_REG_VALID << 5),
    MOD_PCID_REGISTER_PID2 = (PCID_REG_VALID << 6),
    MOD_PCID_REGISTER_PID3 = (PCID_REG_VALID << 7),
    MOD_PCID_REGISTER_CID0 = (PCID_REG_VALID << 8),
    MOD_PCID_REGISTER_CID1 = (PCID_REG_VALID << 9),
    MOD_PCID_REGISTER_CID2 = (PCID_REG_VALID << 10),
    MOD_PCID_REGISTER_CID3 = (PCID_REG_VALID << 11),
    MOD_PCID_REGISTER_ALL = 0xFFF,
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
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PCID_H */
