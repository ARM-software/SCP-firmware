/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_clock.h"

#include <mod_mock_clock.h>

#include <fwk_module.h>

static const struct mod_mock_clock_rate clk0_rate_table[] = {
    { .rate = 1536000 }, { .rate = 2116800 }, { .rate = 2304000 },
    { .rate = 4233600 }, { .rate = 4608000 }, { .rate = 8467200 },
    { .rate = 9216000 },
};

static const struct mod_mock_clock_rate clk1_rate_table[] = {
    { .rate = 2116800 },
};

static const struct mod_mock_clock_rate clk2_rate_table[] = {
    { .rate = 2304000 },
};

static const struct mod_mock_clock_rate clk3_rate_table[] = {
    { .rate = 4233600 },
};

static const struct fwk_element element_table[] = {
    [MOD_MOCK_CLOCK_ELEMENT_IDX_0] =
        {
            .name = "MOCK_CLK0",
            .data = &(const struct mod_mock_clock_element_cfg){
                    .rate_table = clk0_rate_table,
                    .rate_count = FWK_ARRAY_SIZE(clk0_rate_table),
                    .default_rate = 1536000,
                },
        },
    [MOD_MOCK_CLOCK_ELEMENT_IDX_1] =
        {
            .name = "MOCK_CLK1",
            .data = &(const struct mod_mock_clock_element_cfg){
                    .rate_table = clk1_rate_table,
                    .rate_count = FWK_ARRAY_SIZE(clk1_rate_table),
                    .default_rate = 2116800,
                },
        },
    [MOD_MOCK_CLOCK_ELEMENT_IDX_2] =
        {
            .name = "MOCK_CLK2",
            .data = &(const struct mod_mock_clock_element_cfg){
                    .rate_table = clk2_rate_table,
                    .rate_count = FWK_ARRAY_SIZE(clk2_rate_table),
                    .default_rate = 2304000,
                },
        },
    [MOD_MOCK_CLOCK_ELEMENT_IDX_3] =
        {
            .name = "MOCK_CLK3",
            .data = &(const struct mod_mock_clock_element_cfg){
                    .rate_table = clk3_rate_table,
                    .rate_count = FWK_ARRAY_SIZE(clk3_rate_table),
                    .default_rate = 4233600,
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
