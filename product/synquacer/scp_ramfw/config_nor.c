/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(DEVICE_NOR_MX25)
#    include "nor/device_nor_mx25.h"
#elif defined(DEVICE_NOR_MT25)
#    include "nor/device_nor_mt25.h"
#elif defined(DEVICE_NOR_S25)
#    include "nor/device_nor_s25.h"
#elif defined(DEVICE_NOR_W25)
#    include "nor/device_nor_w25.h"
#else
#    error \
        "not found correct definition, please check DEVICE_NOR in src/device.mk."
#endif

#include "mod_hsspi.h"
#include "mod_nor.h"
#include "qspi_api.h"

#include <fwk_module.h>

#include <stddef.h>

/* Configuration of the NOR module. */
static struct qspi_command nor_command_table[MOD_NOR_COMMAND_COUNT] = {
    [MOD_NOR_COMMAND_RESET_ENABLE]         = COMMAND_RESET_ENABLE,
    [MOD_NOR_COMMAND_RESET_EXECUTE]        = COMMAND_RESET_MEMORY,
    [MOD_NOR_COMMAND_READ_ID]              = COMMAND_READ_ID,
    [MOD_NOR_COMMAND_READ_SFDP]            = COMMAND_READ_SFDP,
    [MOD_NOR_COMMAND_WRITE_ENABLE]         = COMMAND_WRITE_ENABLE,
    [MOD_NOR_COMMAND_WRITE_DISABLE]        = COMMAND_WRITE_DISABLE,
    [MOD_NOR_COMMAND_READ_STATUS]          = COMMAND_READ_STATUS_REG,
    [MOD_NOR_COMMAND_WRITE_STATUS]         = COMMAND_WRITE_STATUS_REG,
    [MOD_NOR_COMMAND_READ]                 = COMMAND_READ,
    [MOD_NOR_COMMAND_FAST_READ]            = COMMAND_FAST_READ,
    [MOD_NOR_COMMAND_FAST_READ_1_1_2]      = COMMAND_FAST_READ_1_1_2,
    [MOD_NOR_COMMAND_FAST_READ_1_2_2]      = COMMAND_FAST_READ_1_2_2,
    [MOD_NOR_COMMAND_FAST_READ_1_1_4]      = COMMAND_FAST_READ_1_1_4,
    [MOD_NOR_COMMAND_FAST_READ_1_4_4]      = COMMAND_FAST_READ_1_4_4,
    [MOD_NOR_COMMAND_READ_4B]              = COMMAND_READ_4B,
    [MOD_NOR_COMMAND_FAST_READ_4B]         = COMMAND_FAST_READ_4B,
    [MOD_NOR_COMMAND_FAST_READ_1_1_2_4B]   = COMMAND_FAST_READ_1_1_2_4B,
    [MOD_NOR_COMMAND_FAST_READ_1_2_2_4B]   = COMMAND_FAST_READ_1_2_2_4B,
    [MOD_NOR_COMMAND_FAST_READ_1_1_4_4B]   = COMMAND_FAST_READ_1_1_4_4B,
    [MOD_NOR_COMMAND_FAST_READ_1_4_4_4B]   = COMMAND_FAST_READ_1_4_4_4B,
    [MOD_NOR_COMMAND_PROGRAM]              = COMMAND_PROGRAM,
    [MOD_NOR_COMMAND_PROGRAM_1_1_2]        = COMMAND_PROGRAM_1_1_2,
    [MOD_NOR_COMMAND_PROGRAM_1_2_2]        = COMMAND_PROGRAM_1_2_2,
    [MOD_NOR_COMMAND_PROGRAM_1_1_4]        = COMMAND_PROGRAM_1_1_4,
    [MOD_NOR_COMMAND_PROGRAM_1_4_4]        = COMMAND_PROGRAM_1_4_4,
    [MOD_NOR_COMMAND_PROGRAM_4B]           = COMMAND_PROGRAM_4B,
    [MOD_NOR_COMMAND_PROGRAM_1_1_4_4B]     = COMMAND_PROGRAM_1_1_4_4B,
    [MOD_NOR_COMMAND_PROGRAM_1_4_4_4B]     = COMMAND_PROGRAM_1_4_4_4B,
    [MOD_NOR_COMMAND_ERASE_SECTOR_4KB]     = COMMAND_ERASE_4KB,
    [MOD_NOR_COMMAND_ERASE_SECTOR_32KB]    = COMMAND_ERASE_32KB,
    [MOD_NOR_COMMAND_ERASE_BLOCK]          = COMMAND_ERASE_BLOCK,
    [MOD_NOR_COMMAND_ERASE_CHIP]           = COMMAND_ERASE_CHIP,
    [MOD_NOR_COMMAND_ERASE_SECTOR_4KB_4B]  = COMMAND_ERASE_4KB_4B,
    [MOD_NOR_COMMAND_ERASE_SECTOR_32KB_4B] = COMMAND_ERASE_32KB_4B,
    [MOD_NOR_COMMAND_ERASE_BLOCK_4B]       = COMMAND_ERASE_BLOCK_4B,
};

struct mod_nor_operation nor_operation = {
    .set_io_protocol = nor_set_io_protocol,
    .set_4byte_addr_mode = nor_set_4byte_addr_mode,
    .get_program_result = nor_get_program_result,
    .get_erase_result = nor_get_erase_result,
};

static struct fwk_element nor_dev_table[] = {
    {
        .name = "NOR",
        .data = &((struct mod_nor_dev_config){
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_HSSPI, 0),
            .api_id =
                FWK_ID_API_INIT(FWK_MODULE_IDX_HSSPI, QSPI_API_TYPE_DEFAULT),
            .enable_reset_on_boot = true,
            .program_page_size = PROGRAM_PAGE_SIZE,
            .erase_block_size = ERASE_BLOCK_SIZE,
            .command_table = nor_command_table,
            .operation = &nor_operation,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *nor_get_element_table(fwk_id_t module_id)
{
    return nor_dev_table;
}

const struct fwk_module_config config_nor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(nor_get_element_table),

};
