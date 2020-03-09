/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_JUNO_CDCEL937_H
#define MOD_JUNO_CDCEL937_H

#include "juno_clock.h"

#include <mod_clock.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupJUNOCDCEL937 JUNO CDCEL937
 *
 * @{
 */

/*! Define whether the output Y1 is used */
#define USE_OUTPUT_Y1   0

/*! Output type */
enum mod_juno_cdcel937_output_id {
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y1 = 1, /* Output indices start from 1 */
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y2,
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y3,
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y4,
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y5,
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y6,
    MOD_JUNO_CDCEL937_OUTPUT_ID_Y7,
    MOD_JUNO_CDCEL937_OUTPUT_TYPE_COUNT,
};

/*!
 * \brief Juno CDCEL937 device configuration
 */
struct mod_juno_cdcel937_dev_config {
    /*! Address of the I2C slave device */
    uint8_t slave_address;

    /*! Crystal oscillator input in MHz */
    uint32_t xin_mhz;

    /*! Identifier of the clock HAL */
    fwk_id_t clock_hal_id;

    /*! Identifier of the clock driver input API */
    fwk_id_t clock_api_id;

    /*!
     * The slowest rate the PLL can be set to. Note that this may be
     * higher than the hardware-imposed limit.
     */
    uint64_t min_rate;

    /*!
     * The fastest rate the PLL can be set to. Note that this may be
     * lower than the hardware-imposed limit.
     */
    uint64_t max_rate;

    /*!
     * The maximum precision that can be used when setting the PLL rate. This
     * may be lower than the hardware-imposed limit.
     */
    uint64_t min_step;

    /*! Rate type */
    enum mod_clock_rate_type rate_type;

    /*! Output type */
    enum mod_juno_cdcel937_output_id output_id;

    /*! Lookup table */
    struct juno_clock_lookup *lookup_table;

    /*! Number of entries */
    unsigned int lookup_table_count;
};

/*! Juno CDCEL937 module config */
struct mod_juno_cdcel937_config {
    /*! Identifier of the I2C HAL */
    fwk_id_t i2c_hal_id;
};

/*! API indices */
enum mod_juno_cdcel937_api_idx {
    MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER,
    MOD_JUNO_CDCEL937_API_IDX_HDLCD_DRIVER,
    MOD_JUNO_CDCEL937_API_COUNT,
};

/*! Driver API Identifier */
static const fwk_id_t mod_juno_cdcel937_api_id_clock_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
        MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER);

static const fwk_id_t mod_juno_cdcel937_api_id_hdlcd_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
        MOD_JUNO_CDCEL937_API_IDX_HDLCD_DRIVER);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_JUNO_CDCEL937_H */
