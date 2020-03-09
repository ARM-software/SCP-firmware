/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <f_uart3.h>

#include <mod_f_uart3.h>
#include <mod_log.h>

#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stddef.h>

static const struct mod_f_uart3_device_config **device_config_table;

static struct {
    unsigned stdio_output_ch : 2;
    unsigned parity_enable_flag : 1;
    unsigned even_parity_flag : 1;
    unsigned newline_code : 2;
    uint8_t divider_latch_table_index;
} uart_config = { .stdio_output_ch = 0,
                  .parity_enable_flag = false,
                  .even_parity_flag = false,
                  .newline_code = F_UART3_NEWLINE_CODE_CRLF,
                  .divider_latch_table_index =
                      F_UART3_SYSPARAM_BAUD_RATE_115200 };

static const struct {
    uint8_t DLM;
    uint8_t DLL;
} uart_divider_latch_table[] = {
    [F_UART3_SYSPARAM_BAUD_RATE_9600] = {
       .DLM = F_UART3_DLM_9600,
       .DLL = F_UART3_DLL_9600,
    },
    [F_UART3_SYSPARAM_BAUD_RATE_19200] = {
       .DLM = F_UART3_DLM_19200,
       .DLL = F_UART3_DLL_19200,
    },
    [F_UART3_SYSPARAM_BAUD_RATE_38400] = {
        .DLM = F_UART3_DLM_38400,
        .DLL = F_UART3_DLL_38400,
    },
    [F_UART3_SYSPARAM_BAUD_RATE_57600] = {
        .DLM = F_UART3_DLM_57600,
        .DLL = F_UART3_DLL_57600,
    },
    [F_UART3_SYSPARAM_BAUD_RATE_115200] = {
        .DLM = F_UART3_DLM_115200,
        .DLL = F_UART3_DLL_115200,
    },
    [F_UART3_SYSPARAM_BAUD_RATE_230400] = {
    .DLM = F_UART3_DLM_230400,
    .DLL = F_UART3_DLL_230400,
    }
};

void get_device_reg(
    fwk_id_t device_id,
    struct f_uart3_reg **reg,
    struct f_uart3_dla_reg **dla_reg)
{
    unsigned int device_idx;

    device_idx = fwk_id_get_element_idx(device_id);
    *reg = (struct f_uart3_reg *)device_config_table[device_idx]->reg_base;
    *dla_reg =
        (struct f_uart3_dla_reg *)device_config_table[device_idx]->dla_reg_base;
}

/*
 * Module log driver API
 */

static int do_putchar(fwk_id_t device_id, char c)
{
    struct f_uart3_reg *reg;
    struct f_uart3_dla_reg *dla_reg;

    get_device_reg(device_id, &reg, &dla_reg);
    if (reg != NULL) {
        while ((reg->LSR & F_UART3_LSR_THRE) == 0x0) {
            /* wait for TxFIFO to empty. */
            continue;
        }
        reg->RFR_TFR = (uint32_t)c;
        __DSB();
    }

    return FWK_SUCCESS;
}

static int do_flush(fwk_id_t device_id)
{
    struct f_uart3_reg *reg;
    struct f_uart3_dla_reg *dla_reg;

    get_device_reg(device_id, &reg, &dla_reg);
    if (reg != NULL) {
        while ((reg->LSR & F_UART3_LSR_TEMT) == 0x0) {
            /* wait for TxFIFO to empty. */
            continue;
        }
    }

    return FWK_SUCCESS;
}

static const struct mod_log_driver_api driver_api = {
    .flush = do_flush,
    .putchar = do_putchar,
};

/*
 * Framework handlers
 */
static int f_uart3_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0)
        return FWK_E_DATA;

    /*
     * Create an array of pointers used to store the configuration data pointer
     * of each element.
     */
    device_config_table =
        fwk_mm_calloc(element_count, sizeof(*device_config_table));

    return FWK_SUCCESS;
}

static int f_uart3_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct f_uart3_reg *reg;
    struct f_uart3_dla_reg *dla_reg;
    const struct mod_f_uart3_device_config *config = data;

    reg = (struct f_uart3_reg *)config->reg_base;
    dla_reg = (struct f_uart3_dla_reg *)config->dla_reg_base;
    if ((reg == NULL) || (dla_reg == NULL))
        return FWK_E_DATA;

    /* Set DLAB */
    reg->LCR |= F_UART3_LCR_DLAB;
    __DSB();

    dla_reg->DLL =
        uart_divider_latch_table[uart_config.divider_latch_table_index].DLL;
    dla_reg->DLM =
        uart_divider_latch_table[uart_config.divider_latch_table_index].DLM;
    __DSB();

    /* Clear DLAB */
    reg->LCR &= ~F_UART3_LCR_DLAB;
    __DSB();

    reg->LCR = F_UART3_LCR_WLS_8;
    if (uart_config.parity_enable_flag) {
        reg->LCR |= F_UART3_LCR_PEN;
        if (uart_config.even_parity_flag)
            reg->LCR |= F_UART3_LCR_EPS;
    }
    __DSB();

    /* Disable interrupt */
    reg->IER = 0x0;
    __DSB();

    /* FIFO Reset */
    reg->IIR_FCR = (F_UART3_FCR_RXFRST | F_UART3_FCR_TXFRST);
    __DSB();

    device_config_table[fwk_id_get_element_idx(element_id)] = config;

    return FWK_SUCCESS;
}

static int f_uart3_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &driver_api;

    return FWK_SUCCESS;
}

static int f_uart3_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_f_uart3 = {
    .name = "F_UART3",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = f_uart3_init,
    .element_init = f_uart3_element_init,
    .start = f_uart3_start,
    .process_bind_request = f_uart3_process_bind_request,
};
