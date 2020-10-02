/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Core Configuration Protocol Support.
 */

#ifndef MOD_SCMI_APCORE_H
#define MOD_SCMI_APCORE_H

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMI_APCORE SCMI Core Configuration Protocol
 * \{
 */

/*!
 * \brief SCMI APCORE protocol
 */
#define MOD_SCMI_PROTOCOL_ID_APCORE UINT32_C(0x90)

/*!
 * \brief SCMI APCORE protocol version
 */
#define MOD_SCMI_PROTOCOL_VERSION_APCORE UINT32_C(0x10000)

/*!
 * \brief Identifiers of the SCMI Core Configuration Protocol commands
 */
enum mod_scmi_apcore_command_id {
    MOD_SCMI_APCORE_RESET_ADDRESS_SET = 0x3,
    MOD_SCMI_APCORE_RESET_ADDRESS_GET = 0x4,
};

/*!
 * \brief Platform reset register widths.
 */
enum mod_scmi_apcore_register_width {
    /*! Single-word, 32-bit reset address registers supported */
    MOD_SCMI_APCORE_REG_WIDTH_32,

    /*! Double-word, 64-bit reset address registers supported */
    MOD_SCMI_APCORE_REG_WIDTH_64,

    /*! Number of valid register widths */
    MOD_SCMI_APCORE_REG_WIDTH_COUNT,
};

/*!
 * \brief Reset register group.
 *
 * \details Describes a set of reset registers that are contiguous in memory.
 */
struct mod_scmi_apcore_reset_register_group {
    /*! Address of the first register in the group */
    uintptr_t base_register;

    /*! The number of registers in the group */
    size_t register_count;
};

/*!
 * \brief Module configuration.
 */
struct mod_scmi_apcore_config {
    /*!
     * \brief Pointer to the table of ::mod_scmi_apcore_reset_register_group
     *     structures that define the reset registers within the platform.
     */
    const struct mod_scmi_apcore_reset_register_group
        *reset_register_group_table;

    /*!
     * \brief Number of ::mod_scmi_apcore_reset_register_group structures in
     *     ::mod_scmi_apcore_config::reset_register_group_table.
     */
    size_t reset_register_group_count;

    /*! Width of the reset address supported by the platform */
    enum mod_scmi_apcore_register_width reset_register_width;
};

/*!
 * \}
 */

#endif /* MOD_SCMI_APCORE_H */
