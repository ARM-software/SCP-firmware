/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_HSSPI_API_H
#define INTERNAL_HSSPI_API_H

#include <internal/hsspi_driver.h>

#include <stdbool.h>
#include <stdint.h>

struct HSSPI_clk_config {
    HSSPI_EN_MCTRL_CDSS_t clk_sel;
    int clk_div;
    int syncon;
};

/**
 * Initialize HS-SPI controller and external serial flash memory
 */
void HSSPI_init(void);
/**
 * Finalize HS-SPI controller and external serial flash memory
 */
void HSSPI_exit(void);

#endif /* INTERNAL_HSSPI_API_H */
