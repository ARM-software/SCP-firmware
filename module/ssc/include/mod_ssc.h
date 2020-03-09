/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SSC_H
#define MOD_SSC_H

#include <stdint.h>

/*!
 * \brief Module configuration.
 */
struct mod_ssc_config {
    /*! Base address of the SSC registers. */
    uintptr_t ssc_base;

    /*! Debug authentication configuration set register */
    uint32_t ssc_debug_cfg_set;

    /*! Product name */
    const char *product_name;
};

/*!
 * \brief Module API indicies.
 */
enum mod_ssc_api_idx {
    MOD_SSC_SYSTEM_INFO_DRIVER_DATA_API_IDX,
    MOD_SSC_API_COUNT
};

#endif /* MOD_SSC_H */
