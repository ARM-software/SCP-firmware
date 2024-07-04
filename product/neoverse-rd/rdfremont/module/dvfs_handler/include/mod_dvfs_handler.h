/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Structure definitions for the DVFS handler module.
 */

#ifndef DVFS_HANDLER_H
#define DVFS_HANDLER_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief Operating Performance Point (OPP).
 */
struct mod_dvfs_handler_opp {
    /*! Level value of the OPP. Cannot be 0 */
    unsigned int level;
    /*! Power supply voltage in microvolts (uV) */
    unsigned int voltage;
    /*! Clock rate in Hertz (Hz) */
    unsigned int frequency;
};

/*!
 * \brief Element configuration data for the DFVS handler.
 */
struct mod_dvfs_handler_element_config {
    /*! Base address of LCP DVFS hardware */
    uint32_t dvfs_frame_addr;

    /*!
     * \brief Operating points.
     *
     * \note The frequencies and levels of these operating points must be in
     *      ascending order.
     */
    struct mod_dvfs_handler_opp *opps;

    /*! number of entries in the opps structure */
    unsigned int opp_count;

    /*! Sustained operating point index */
    unsigned int sustained_idx;

    /*! The max count for checking the DVFS status registers */
    unsigned int status_check_max;

    /*! Transport element for the fastchannel set level */
    fwk_id_t transport_element_id;

    /*! Transport fastchannel API */
    fwk_id_t transport_fch_api_id;
};

#endif /* DVFS_HANDLER_H */
