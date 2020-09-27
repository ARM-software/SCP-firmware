/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_MOCK_PMIC_DRIVER_API_PRIVATE_H
#define MOD_RCAR_MOCK_PMIC_DRIVER_API_PRIVATE_H

#include <mod_rcar_pmic.h>

extern const struct mod_rcar_pmic_driver_api __mod_rcar_mock_pmic_driver_api;

/* Default limits measured in millivolts and milliamps */
#define BD9571MWV_MIN_MV        750
#define BD9571MWV_MAX_MV        1030
#define BD9571MWV_STEP_MV       10

/* Define Register */
#define REG_DATA_DVFS_SetVID_MASK   (0x7EU)
#define REG_ADDR_DVFS_SetVID        (0x54U)
#define SLAVE_ADDR_PMIC             (0x30U)
#define DIV_ROUND(n, d) (((n) + (d) / 2) / (d))

extern int32_t rcar_iic_dvfs_send(uint8_t slave, uint8_t regr, uint8_t data);

#endif /* MOD_RCAR_MOCK_PMIC_DRIVER_API_PRIVATE_H */
