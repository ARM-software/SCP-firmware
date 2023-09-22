/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_FCH_POLLED_H
#define MOD_FCH_POLLED_H

#include <mod_transport.h>

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupPlatform Fast Channel Driver
 * \{
 */

/*! Module name */
#define MOD_NAME "[FCH_POLLED] "

/*! Minimum poll rate in microseconds */
#define FCH_MIN_POLL_RATE_US 4000

/*!
 * \brief Platform FCH Driver module API indicies
 */
enum mod_fch_polled_api_idx {
#ifdef BUILD_HAS_MOD_TRANSPORT
    /*! TRANSPORT driver API */
    MOD_FCH_POLLED_API_IDX_TRANSPORT,
#endif
    /*! Number of APIs */
    MOD_FCH_POLLED_API_IDX_COUNT,
};

/*!
 * \brief Platform FCH Driver Channel configuration
 */
struct mod_fch_polled_channel_config {
    /*! Fast Channel description structure */
    struct mod_transport_fast_channel_addr fch_addr;
};

/*!
 * \brief Platform FCH module configuration
 */
struct mod_fch_polled_config {
    /*! Fast Channel alarm ID */
    fwk_id_t fch_alarm_id;

    /*! Fast Channel polling rate */
    uint32_t fch_poll_rate;

    /*! Fast channel rate limit */
    uint32_t rate_limit;

    /*! Attributes */
    uint32_t attributes;
};

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_FCH_POLLED_H */
