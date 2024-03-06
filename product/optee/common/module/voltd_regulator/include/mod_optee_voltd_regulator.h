/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_OPTEE_VOLTD_REGULATOR_H
#define MOD_OPTEE_VOLTD_REGULATOR_H

#include <stdbool.h>

/* Opaque struct for OP-TEE voltage regualtor device reference */
struct optee_regulator;

/*!
 * \brief Platform regulator configuration.
 */
struct mod_optee_voltd_regulator_dev_config {
    /*! OP-TEE voltage regulator or NULL is not exposed by SCMI server */
    struct optee_regulator *regulator;
    /*! True if regulator should be enabled at SCMI server boot time */
    bool default_enabled;
};

#endif /* MOD_OPTEE_VOLTD_REGULATOR_H */
