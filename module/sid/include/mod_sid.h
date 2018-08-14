/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SID_H
#define MOD_SID_H

#include <stdint.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>
#include <mod_pcid.h>

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
 * \brief Module API indicies.
 */
enum mod_sid_api_idx {
    MOD_SID_API_IDX_INFO, /*!< Index of the info api. */
    MOD_SID_API_COUNT,    /*!< Number of apis. */
};

/*!
 * \brief Info API id.
 */
static const fwk_id_t mod_sid_api_id_info =
    FWK_ID_API_INIT(FWK_MODULE_IDX_SID, MOD_SID_API_IDX_INFO);

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

    /*! ID for the node when there are multiple sockets */
    unsigned int node_id;

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
struct mod_sid_api_info {
    /*!
     * \brief Get a pointer to the structure holding the system information.
     *
     * \return Pointer to system information structure.
     */
    const struct mod_sid_info * (*get_system_info)(void);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SID_H */
