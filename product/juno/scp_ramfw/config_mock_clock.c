/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_clock.h"

#include <mod_mock_clock.h>

#include <fwk_macros.h>
#include <fwk_module.h>

static const struct mod_mock_clock_rate i2s_rate_table[] = {
    { .rate = 1536000 }, { .rate = 2116800 }, { .rate = 2304000 },
    { .rate = 4233600 }, { .rate = 4608000 }, { .rate = 8467200 },
    { .rate = 9216000 },
};

static const struct fwk_element element_table[] = {
    [MOD_MOCK_CLOCK_ELEMENT_IDX_I2S] =
        {
            .name = "MOCK_I2SCLK",
            .data = &(const struct mod_mock_clock_element_cfg){
                    .rate_table = i2s_rate_table,
                    .rate_count = FWK_ARRAY_SIZE(i2s_rate_table),
                    .default_rate = 1536000,
                },
        },
    [MOD_MOCK_CLOCK_ELEMENT_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mock_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
