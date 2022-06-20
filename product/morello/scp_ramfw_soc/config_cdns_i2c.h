/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_CDNS_I2C_H
#define CONFIG_CDNS_I2C_H

enum config_cdns_i2c_element_idx {
    CONFIG_CDNS_I2C_ELEMENT_IDX_SPD,
    CONFIG_CDNS_I2C_ELEMENT_IDX_C2C,
    CONFIG_CDNS_I2C_ELEMENT_IDX_PMIC_OSC,
    CONFIG_CDNS_I2C_ELEMENT_IDX_PCC,
    CONFIG_CDNS_I2C_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_CDNS_I2C_H */
