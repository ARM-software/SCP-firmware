/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_GPIO_H
#define INTERNAL_GPIO_H

#include "synquacer_mmap.h"

#include <stdint.h>

#define PRMUX_MAX_IDX CONFIG_SOC_PRMUX_MAX_IDX
#define GPIO_MAX_IDX 3

void fw_gpio_init(void);
void prmux_set_pingrp(void *prmux_base_addr, uint32_t idx, uint32_t pingrp);
void gpio_set_data(void *gpio_base_addr, uint32_t idx, uint8_t value);
uint8_t gpio_get_data(void *gpio_base_addr, uint32_t idx);
void gpio_set_direction(void *gpio_base_addr, uint32_t idx, uint8_t value);
uint8_t gpio_get_direction(void *gpio_base_addr, uint32_t idx);
void gpio_set_function(void *gpio_base_addr, uint32_t idx, uint8_t value);
uint8_t gpio_get_function(void *gpio_base_addr, uint32_t idx);

#endif /* INTERNAL_GPIO_H */
