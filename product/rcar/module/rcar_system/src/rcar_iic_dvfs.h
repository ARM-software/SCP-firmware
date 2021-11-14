/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* The use of "subordinate" may not be in sync with platform documentation */

#ifndef RCAR_IIC_DVFS_H
#define RCAR_IIC_DVFS_H

/* PMIC subordinate */
#define PMIC          (0x30)
#define BKUP_MODE_CNT (0x20)
#define DVFS_SET_VID (0x54)
#define REG_KEEP10 (0x79)

/* EEPROM subordinate */
#define EEPROM   (0x50)
#define BOARD_ID (0x70)

int32_t rcar_iic_dvfs_receive(uint8_t subordinate, uint8_t reg, uint8_t *data);
int32_t rcar_iic_dvfs_send(uint8_t subordinate, uint8_t regr, uint8_t data);

#endif /* RCAR_IIC_DVFS_H */
