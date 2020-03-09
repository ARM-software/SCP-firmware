/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SID_H
#define MOD_SID_H

#include <mod_pcid.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleSID System Identification (SID)
 *
 * \brief Module used to interface with the SID register set.
 *
 * \details This module uses the SID register set to get information about the
 *     subsystem that the firmware is running on. @{
 */

/*!
 * \brief System information
 */
struct mod_sid_info {
    /*! Major revision number of the subsystem */
    unsigned int system_major_revision;

    /*! Minor revision number of the subsystem */
    unsigned int system_minor_revision;

    /*! Designer ID of the subsystem */
    unsigned int system_designer_id;

    /*! Part number of the subsystem */
    unsigned int system_part_number;

    /*! Major revision number of the SoC */
    unsigned int soc_major_revision;

    /*! Minor revision number of the SoC */
    unsigned int soc_minor_revision;

    /*! Designer ID of the SoC */
    unsigned int soc_designer_id;

    /*! Part number of the SoC */
    unsigned int soc_part_number;

    /*! Multi-chip mode tie-off value - enabled or disabled */
    bool multi_chip_mode;

    /*! Node number indicating the chip id in multi socket system */
    uint8_t node_number;

    /*! Configuration number of the subsystem */
    unsigned int config_number;

    /*! Name of the subsystem */
    const char *name;

    /*! Element index of the subsystem */
    unsigned int system_idx;
};

/*!
 * \brief Module configuration.
 */
struct mod_sid_config {
    /*! Base address of the SID registers. */
    uintptr_t sid_base;

    /*! Expected values of the PID and CID registers */
    struct mod_pcid_registers pcid_expected;
};

/*!
 * \brief Subsystem configuration.
 */
struct mod_sid_subsystem_config {
    unsigned int part_number; /*!< Part number of the subsystem */
};

/*!
 * \brief Module interface.
 */

/*!
 * \brief Get a pointer to the structure holding the system information.
 *
 * \param[out] system_info Pointer to the system information data.
 *
 * \retval FWK_SUCCESS The pointer was returned successfully.
 * \retval FWK_E_INIT The system information is not initialized.
 */
int mod_sid_get_system_info(const struct mod_sid_info **system_info);

/*!
 * \brief Module API indices.
 */
enum mod_sid_api_idx {
    MOD_SID_SYSTEM_INFO_DRIVER_DATA_API_IDX,
    MOD_SID_API_COUNT
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SID_H */
