/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_AMU_SMCF_DRV_H
#define MOD_AMU_SMCF_DRV_H

#include <fwk_id.h>

#define AMU_TAG_BUFFER_SIZE 16

/*!
 * \brief amu_smcf_drv element config.
 *
 * \details The configuration data for each element which
 *      corresponds to a core that can have counters as
 *      subelements.
 */
struct amu_smcf_drv_element_config {
    /*! SMCF MLI id for the element */
    fwk_id_t smcf_mli_id;

    /*! Total number of counters */
    uint32_t total_num_of_counters;

    /*! Table of counter offsets per core. */
    uint32_t *counter_offsets;
};

/*!
 * \brief Indexes of the interfaces exposed by the module.
 */
enum mod_amu_smcf_drv_api_idx {
    /*! Data related API's */
    MOD_AMU_SMCF_DRV_API_IDX_DATA,

    /*! Number of exposed interfaces */
    MOD_AMU_SMCF_DRV_API_IDX_COUNT,
};

#endif /* MOD_AMU_SMCF_DRV_H */
