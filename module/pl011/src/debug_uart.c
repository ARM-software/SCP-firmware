/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dbg_system_map.h>
#include <pl011.h>

#include <mod_pl011.h>

#include <fwk_assert.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const struct mod_pl011_device_config config = {
    .reg_base = DEBUG_UART_BASE,
    .baud_rate_bps = 115200,
    .clock_rate_hz = 24 * FWK_MHZ
};

uint32_t cli_platform_uart_init(void)
{
    volatile struct pl011_reg *reg;
    int status;

    reg = (volatile struct pl011_reg *)config.reg_base;
    if (reg == NULL)
        return FWK_E_DATA;

    status = mod_pl011_set_baud_rate(config.baud_rate_bps,
                           config.clock_rate_hz,
                           config.reg_base);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Initialize PL011 device
     */
    reg->ECR = PL011_ECR_CLR;
    reg->LCR_H = PL011_LCR_H_WLEN_8BITS |
                 PL011_LCR_H_FEN;
    reg->CR = PL011_CR_UARTEN |
              PL011_CR_RXE |
              PL011_CR_TXE;

    return FWK_SUCCESS;
}

uint32_t cli_platform_uart_get(char *c, const bool block)
{
    volatile struct pl011_reg *reg;
    uint32_t ch = 0;

    reg = (volatile struct pl011_reg *)config.reg_base;

    while (1) {
        /* Wait for a character to become available if block is set. */
        if ((reg->FR & PL011_FR_RXFE) == 0) {
            ch = reg->DR;
            *c = (char)ch;
            return FWK_SUCCESS;
        } else if (block == false) {
            return FWK_E_DATA;
        }
    }
}

uint32_t cli_platform_uart_put(const char *c, bool block)
{
    int status;
    volatile struct pl011_reg *reg;

    reg = (volatile struct pl011_reg *)config.reg_base;

    uint32_t ch = (uint32_t)*c;

    /* If we get a \n, send a \r first. */
    if (*c == '\n') {
        status = cli_platform_uart_put("\r", true);

        if (status != FWK_SUCCESS)
            return status;
    }

    do {
        /* Wait for UART to be ready for a character if block is set. */
        if ((reg->FR & PL011_FR_TXFF) == 0) {
            reg->DR = ch;
            return FWK_SUCCESS;
        }
    } while (block);

    return FWK_E_DATA;
}
