/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sysdef_option.h>

#include <internal/nic400.h>

#include <fwk_macros.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SYNQUACER_CHIP_VER_MP UINT32_C(1)

static const uint32_t nic_config_mp[NIC_CONFIG_NUM] = {
    1, NIC_SETUP_SKIP, NIC_SETUP_SKIP, 1, 0x7, END_OF_NIC_LIST
};

static struct sysdef_option {
    uint32_t clear_clkforce; /* Value to be set to CLKFORCE_SET register */
    const uint32_t *scbm_mv_nic_config;
    char *chip_version;
    const struct sysdef_option_gpio_desc *gpio_desc_p;
    uint32_t gpio_desc_num;
    bool ap_reboot_enable;
    uint32_t i2c_for_spd_read_addr;
    uint32_t sensor_num;
} sysdef_option;

static const struct sysdef_option_gpio_desc gpio_desc_synquacer_mp[] = {
    { 14 /* pin_no */, true /* inv */, "pcie1-ep-detected" /* str */ }
};

uint32_t sysdef_option_get_clear_clkforce(void)
{
    return sysdef_option.clear_clkforce;
}

bool sysdef_option_get_ap_reboot_enable(void)
{
    return sysdef_option.ap_reboot_enable;
}

const uint32_t *sysdef_option_get_scbm_mv_nic_config(void)
{
    return sysdef_option.scbm_mv_nic_config;
}

char *sysdef_option_get_chip_version(void)
{
    return sysdef_option.chip_version;
}

uint32_t sysdef_option_get_gpio_desc(
    const struct sysdef_option_gpio_desc **gpio_desc_pp)
{
    if (gpio_desc_pp == NULL)
        return 0;

    *gpio_desc_pp = sysdef_option.gpio_desc_p;
    return sysdef_option.gpio_desc_num;
}

uint32_t sysdef_option_get_i2c_for_spd_read_addr(void)
{
    return sysdef_option.i2c_for_spd_read_addr;
}

#define CMN_ST2_OFFSET UINT32_C(0x7b4)

uint32_t fw_get_chip_ver(void)
{
    return SYNQUACER_CHIP_VER_MP;
}

uint32_t sysdef_option_get_sensor_num(void)
{
    return sysdef_option.sensor_num;
}

void sysdef_option_init_synquacer(void)
{
    sysdef_option.clear_clkforce = 0x144U;
    sysdef_option.scbm_mv_nic_config = nic_config_mp;
    sysdef_option.chip_version = "2";
    sysdef_option.gpio_desc_p = gpio_desc_synquacer_mp;
    sysdef_option.gpio_desc_num = FWK_ARRAY_SIZE(gpio_desc_synquacer_mp);
    sysdef_option.ap_reboot_enable = false;
    sysdef_option.i2c_for_spd_read_addr = I2C_MP_START_ADDR;
    sysdef_option.sensor_num = 7;
}
