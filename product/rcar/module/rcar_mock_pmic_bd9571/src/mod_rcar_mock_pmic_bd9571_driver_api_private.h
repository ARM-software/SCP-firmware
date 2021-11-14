/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* The use of "subordinate" may not be in sync with platform documentation */

#ifndef MOD_RCAR_MOCK_PMIC_BD9571_DRIVER_API_PRIVATE_H
#define MOD_RCAR_MOCK_PMIC_BD9571_DRIVER_API_PRIVATE_H

#include <mod_rcar_pmic.h>
#include <mod_psu.h>

extern const struct mod_rcar_pmic_driver_api __mod_rcar_mock_pmic_driver_api;

/* Default limits measured in millivolts and milliamps */
#define BD9571MWV_MIN_MV 750
#define BD9571MWV_MAX_MV 1030
#define BD9571MWV_STEP_MV 10

/* Define Register */
#define REG_DATA_DVFS_SetVID_MASK (0x7EU)
#define REG_ADDR_DVFS_SetVID      (0x54U)
#define SUBORDINATE_ADDR_PMIC     (0x30U)
#define DIV_ROUND(n, d)           (((n) + (d) / 2) / (d))
#define REG_ADDR_DDR_BKUP         (0x20U)
#define REG_DATA_DDR_BKUP_ON      (0xF)
#define REG_DATA_DDR_BKUP_OFF     (0x0)

extern int32_t rcar_iic_dvfs_send(
    uint8_t subordinate,
    uint8_t regr,
    uint8_t data);

#endif /* MOD_RCAR_MOCK_PMIC_BD9571_DRIVER_API_PRIVATE_H */
