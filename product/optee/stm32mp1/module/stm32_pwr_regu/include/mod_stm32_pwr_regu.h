/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_STM32_PWR_REGU_H
#define MOD_STM32_PWR_REGU_H

/*!
 * \brief Platform clocks configuration.
 */
struct mod_stm32_pwr_regu_dev_config {
    unsigned long pwr_id;
    const char *regu_name;
};

#endif /* MOD_STM32_PWR_REGU_H */
