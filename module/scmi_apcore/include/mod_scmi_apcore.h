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
     * \brief Pointer to the table of \ref mod_scmi_apcore_reset_register_group
     *     structures that define the reset registers within the platform.
     */
    const struct mod_scmi_apcore_reset_register_group
        *reset_register_group_table;

    /*!
     * \brief Number of \ref mod_scmi_apcore_reset_register_group structures in
     *     \ref reset_register_group_table.
     */
    size_t reset_register_group_count;

    /*! Width of the reset address supported by the platform */
    enum mod_scmi_apcore_register_width reset_register_width;
};

/*!
 * \}
 */

#endif /* MOD_SCMI_APCORE_H */
