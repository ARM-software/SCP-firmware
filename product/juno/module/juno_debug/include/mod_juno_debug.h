/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno Debug module.
 */

#ifndef MOD_JUNO_DEBUG_H
#define MOD_JUNO_DEBUG_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \brief Element-specific clock configuration.
 */
struct juno_css_debug_dev {
    /*! Divider for ATCLK */
    uint32_t div_atclk;

    /*! Divider for TRACECLK */
    uint32_t div_traceclk;

    /*! Divider for PCLK */
    uint32_t div_pclk;

    /*! Flag indicating whether manual reset is required */
    bool manual_reset_required;
};

/*!
 * \brief Element configuration.
 */
struct mod_juno_debug_dev_config {
    /*! Pointer to device-specific clock settings */
    struct juno_css_debug_dev *clk_settings;

    /*! Identifier of the DBGSYS power domain */
    fwk_id_t pd_dbgsys_id;

    /*! Identifier of the BIG_SSTOP power domain */
    fwk_id_t pd_big_sstop_id;

    /*! Identifier of the LITTLE_SSTOP power domain */
    fwk_id_t pd_little_sstop_id;
};

/*!
 * \brief Module configuration.
 */
struct mod_juno_debug_config {
    /*!
     * \brief Identifier of the timer.
     *
     * \details Used for time-out when applying clock settings.
     */
    fwk_id_t timer_id;
};

/*!
 * \brief Juno Debug API indices.
 */
enum mod_juno_debug_api_idx {
    /*! Index of the driver API */
    MOD_JUNO_DEBUG_API_IDX_DRIVER,

    /*! Number of APIs for the Juno Debug driver module */
    MOD_JUNO_DEBUG_API_IDX_COUNT,
};

#endif  /* MOD_JUNO_DEBUG_H */
