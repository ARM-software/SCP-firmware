/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "synquacer_mmap.h"

#include <sysdef_option.h>

#include <internal/gpio.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>
#include <fwk_macros.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

static const uint8_t prmux_pingrp[] = CONFIG_SCB_PRMUX_PINGRP;
static const uint8_t gpio_direction[] = CONFIG_SCB_GPIO_DIRECTION;
static const uint8_t gpio_function[] = CONFIG_SCB_GPIO_FUNCTION;

void prmux_set_pingrp(void *prmux_base_addr, uint32_t idx, uint32_t pingrp)
{
    if (idx > PRMUX_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return;
    }

    writel((uint32_t)prmux_base_addr + (idx << 2), pingrp);
}

void gpio_set_data(void *gpio_base_addr, uint32_t idx, uint8_t value)
{
    if (idx > GPIO_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return;
    }

    writel((uint32_t)gpio_base_addr + (idx << 2), value);
}

uint8_t gpio_get_data(void *gpio_base_addr, uint32_t idx)
{
    if (idx > GPIO_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return 0;
    }

    return readl((uint32_t)gpio_base_addr + (idx << 2));
}

void gpio_set_direction(void *gpio_base_addr, uint32_t idx, uint8_t value)
{
    if (idx > GPIO_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return;
    }

    writel((uint32_t)gpio_base_addr + 0x10 + (idx << 2), value);
}

uint8_t gpio_get_direction(void *gpio_base_addr, uint32_t idx)
{
    if (idx > GPIO_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return 0;
    }

    return readl((uint32_t)gpio_base_addr + 0x10 + (idx << 2));
}

void gpio_set_function(void *gpio_base_addr, uint32_t idx, uint8_t value)
{
    if (idx > GPIO_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return;
    }

    writel((uint32_t)gpio_base_addr + 0x20 + (idx << 2), value);
}

uint8_t gpio_get_function(void *gpio_base_addr, uint32_t idx)
{
    if (idx > GPIO_MAX_IDX) {
        FWK_LOG_ERR("Error@%s idx(%" PRIu32 ") too big", __func__, idx);
        return 0;
    }

    return readl((uint32_t)gpio_base_addr + 0x20 + (idx << 2));
}

void fw_gpio_init(void)
{
    size_t i;
    uint32_t gpio_initial_values;

    uint32_t gpio_desc_num;
    const struct sysdef_option_gpio_desc *gpio_desc_p;

    FWK_LOG_INFO("[SYSTEM] Setting up PRMUX");
    for (i = 0; i < FWK_ARRAY_SIZE(prmux_pingrp); i++) {
        prmux_set_pingrp(
            (void *)CONFIG_SOC_PRMUX_BASE_ADDR, i, prmux_pingrp[i]);
    }
    FWK_LOG_INFO("[SYSTEM] Finished setting up PRMUX");

    FWK_LOG_INFO("[SYSTEM] Setting up GPIO");
    for (i = 0; i < FWK_ARRAY_SIZE(gpio_function); i++)
        gpio_set_function((void *)CONFIG_SOC_AP_GPIO_BASE, i, gpio_function[i]);

    for (i = 0; i < FWK_ARRAY_SIZE(gpio_direction); i++)
        gpio_set_direction(
            (void *)CONFIG_SOC_AP_GPIO_BASE, i, gpio_direction[i]);

    FWK_LOG_INFO("[SYSTEM] Finished setting up GPIO");

    gpio_initial_values =
        (gpio_get_data((void *)CONFIG_SOC_AP_GPIO_BASE, 3) << 24) |
        (gpio_get_data((void *)CONFIG_SOC_AP_GPIO_BASE, 2) << 16) |
        (gpio_get_data((void *)CONFIG_SOC_AP_GPIO_BASE, 1) << 8) |
        gpio_get_data((void *)CONFIG_SOC_AP_GPIO_BASE, 0);

    FWK_LOG_INFO(
        "[SYSTEM] Initial GPIO input values = 0x%08" PRIx32 ": ",
        gpio_initial_values);

    gpio_desc_num = sysdef_option_get_gpio_desc(&gpio_desc_p);
    for (i = 0; i < gpio_desc_num; i++) {
        if (((gpio_initial_values >> gpio_desc_p->pin_no) & 0x1) ==
            (gpio_desc_p->inv ? 1 : 0)) {
            /* prepend "!" if the target signal is not asserted */
            FWK_LOG_INFO("!");
        }
        FWK_LOG_INFO("%s ", gpio_desc_p->str);
    }
}
