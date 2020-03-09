/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_JUNO_HDLCD_H
#define MOD_JUNO_HDLCD_H

#include "juno_clock.h"

#include <mod_clock.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupJUNOHDLCD JUNO HDLCD
 *
 * @{
 */

/*!
 * \brief Juno HDLCD device configuration
 */
struct mod_juno_hdlcd_dev_config {
    /*!
     * \brief Identifier of the driver
     */
    fwk_id_t driver_id;

    /*!
     * \brief Identifier of the driver API
     */
    fwk_id_t driver_api_id;

    /*!
     * \brief Identifier of the clock HAL
     */
    fwk_id_t clock_hal_id;

    /*!
     * \brief Identifier of the clock driver input API
     */
    fwk_id_t clock_api_id;

    /*!
     * \brief SCC control register
     */
    FWK_RW uint32_t *scc_control;

    /*!
     * \brief The lowest rate the clock can be set to
     */
    uint64_t min_rate;

    /*!
     * \brief The highest rate the clock can be set to
     */
    uint64_t max_rate;

    /*!
     * \brief The maximum precision that can be used when setting the clock rate
     */
    uint64_t min_step;

    /*!
     * \brief Rate type
     */
    enum mod_clock_rate_type rate_type;

    /*!
     * \brief Lookup table for the rate and PLL values
     */
    struct juno_clock_lookup *lookup_table;

    /*!
     * \brief Number of entries to the lookup table
     */
    unsigned int lookup_table_count;
};

/*!
 * \brief HDLCD driver API
 *
 * \details This API provides a function to set the rate of a clock given the
 *      index in the lookup table.
 *
 * \note Driver of HDLCD module needs to implement this API.
 */
struct mod_juno_hdlcd_drv_api {
    /*!
     * \brief Change the rate of a clock using its lookup table.
     *
     * \param clock_id The identifier of the clock
     * \param index The index of the lookup table that contains the information
     *      to set the rate of the clock.
     *
     * \retval FWK_SUCCESS The request is successful.
     * \retval FWK_E_PARAM One or more parameters are incorrect.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_rate_from_index)(fwk_id_t clock_id, int index);
};

/*! API indices */
enum mod_juno_hdlcd_api_idx {
    MOD_JUNO_HDLCD_API_IDX_CLOCK_DRIVER,
    MOD_JUNO_HDLCD_API_IDX_HDLCD_DRIVER_RESPONSE,
    MOD_JUNO_HDLCD_API_COUNT,
};

/*! Clock Driver API Identifier */
static const fwk_id_t mod_juno_hdlcd_api_id_clock_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
        MOD_JUNO_HDLCD_API_IDX_CLOCK_DRIVER);

/*! Clock Driver API Identifier */
static const fwk_id_t mod_juno_hdlcd_api_id_hdlcd_driver_response =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
        MOD_JUNO_HDLCD_API_IDX_HDLCD_DRIVER_RESPONSE);
/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_JUNO_HDLCD_H */
