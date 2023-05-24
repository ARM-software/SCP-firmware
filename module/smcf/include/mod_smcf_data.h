/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SMCF Data
 */

#ifndef MOD_SMCF_DATA_H
#define MOD_SMCF_DATA_H

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleSMCF SMCF Driver
 * \{
 */

/*!
 * \brief Sample trigger types
 */
enum mod_smcf_sample_type {
    SMCF_SAMPLE_TYPE_MANUAL,
    SMCF_SAMPLE_TYPE_PERIODIC,
    SMCF_SAMPLE_TYPE_DATA_READ,
    SMCF_SAMPLE_TYPE_TRIGGER_INPUT,
    SMCF_SAMPLE_TYPE_COUNT
};

/*!
 * \brief Data location
 */
enum mod_smcf_data_location {
    SMCF_DATA_LOCATION_MGI,
    SMCF_DATA_LOCATION_RAM,
    SMCF_DATA_LOCATION_ALTERNATE
};

/*!
 * \brief Data header format shift
 */
enum sample_identifier_header_format_shift {
    SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID_SHIFT,
    SMCF_SAMPLE_HEADER_FORMAT_DATA_BITS_VALID_SHIFT,
    SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID_SHIFT,
    SMCF_SAMPLE_HEADER_FORMAT_TAG_ID_SHIFT,
    SMCF_SAMPLE_HEADER_FORMAT_END_ID_SHIFT,
};

/*! Data header group ID bitmask */
#define SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID \
    (1U << SMCF_SAMPLE_HEADER_FORMAT_GROUP_ID_SHIFT)

/*! Data header valid bits bitmask */
#define SMCF_SAMPLE_HEADER_FORMAT_DATA_VALID_BITS \
    (1U << SMCF_SAMPLE_HEADER_FORMAT_DATA_BITS_VALID_SHIFT)

/*! Data header count ID  bitmask */
#define SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID \
    (1U << SMCF_SAMPLE_HEADER_FORMAT_COUNT_ID_SHIFT)

/*! Data header tag ID bitmask */
#define SMCF_SAMPLE_HEADER_FORMAT_TAG_ID \
    (1U << SMCF_SAMPLE_HEADER_FORMAT_TAG_ID_SHIFT)

/*! Data header end ID bitmask */
#define SMCF_SAMPLE_HEADER_FORMAT_END_ID \
    (1U << SMCF_SAMPLE_HEADER_FORMAT_END_ID_SHIFT)

/*! Memory Configuration */
struct mod_smcf_data_config {
    /*! Data header format */
    uint32_t header_format;

    /*! Data location */
    enum mod_smcf_data_location data_location;

    /*!
     * Data write location used by the hardware.
     * Set to zero if data read from MGI.
     * The address must be 32-bit aligned according to the spec.
     */
    uint64_t write_addr;

    /*!
     * Data Read location used by the firmware,
     * Set to zero if data read from MGI.
     */
    uint32_t *read_addr;
};

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_SMCF_DATA_H */
