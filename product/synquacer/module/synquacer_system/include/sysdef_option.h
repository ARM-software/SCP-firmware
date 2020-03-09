/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDEF_OPTION_H
#define SYSDEF_OPTION_H

#include <stdbool.h>
#include <stdint.h>

#define NIC_CONFIG_NUM 6

#define I2C_TEG_START_ADDR 0x51
#define I2C_MP_START_ADDR 0x50

/**
 * GPIO-pin auto descriptor settings
 *
 *   pin_no: GPIO pin #
 *   inv: true if using inverted logic
 *   str: description
 */
/*!
 * \brief GPIO-pin auto descriptor settings
 */
struct sysdef_option_gpio_desc {
    /*! GPIO pin number. */
    int pin_no;
    /*! true if using inverted logic */
    bool inv;
    /*! description. */
    char *str;
};

uint32_t sysdef_option_get_clear_clkforce(void);
bool sysdef_option_get_ap_reboot_enable(void);
const uint32_t *sysdef_option_get_scbm_mv_nic_config(void);
char *sysdef_option_get_chip_version(void);
bool sysdef_option_get_gic500_preits_bug_tweak_enabled(void);
bool sysdef_option_get_ddr_addr_trans_bug_tweak_enabled(void);
uint32_t sysdef_option_get_i2c_for_spd_read_addr(void);
uint32_t sysdef_option_get_sensor_num(void);
uint32_t sysdef_option_get_gpio_desc(
    const struct sysdef_option_gpio_desc **gpio_desc_pp);
void sysdef_option_init_synquacer(void);

#endif /* SYSDEF_OPTION_H */
