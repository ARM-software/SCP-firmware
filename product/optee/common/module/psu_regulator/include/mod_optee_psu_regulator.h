/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, STMicroelectronics and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_OPTEE_PSU_REGULATOR_H
#define MOD_OPTEE_PSU_REGULATOR_H

/* Opaque struct for OP-TEE voltage regualtor device reference */
struct optee_regulator;

/*!
 * \brief Platform regulator configuration.
 */
struct mod_optee_psu_regulator_dev_config {
    struct optee_regulator *regulator;
};

#endif /* MOD_OPTEE_PSU_REGULATOR_H */
