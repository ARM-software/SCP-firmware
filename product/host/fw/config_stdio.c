/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_stdio.h"

#include <mod_stdio.h>

#include <fwk_attributes.h>
#include <fwk_module.h>

#include <stdio.h>

static struct mod_stdio_element_cfg config_stdio_element_cfg[] = {
    [CONFIG_STDIO_ELEMENT_IDX_STDIN] = {
        .type = MOD_STDIO_ELEMENT_TYPE_STREAM,
        .stream = NULL,
    },

    [CONFIG_STDIO_ELEMENT_IDX_STDOUT] = {
        .type = MOD_STDIO_ELEMENT_TYPE_STREAM,
        .stream = NULL,
    },
};

static const struct fwk_element config_stdio_elements[] = {
    [CONFIG_STDIO_ELEMENT_IDX_STDIN] = {
        .name = "stdin",
        .data = &config_stdio_element_cfg[CONFIG_STDIO_ELEMENT_IDX_STDIN],
    },

    [CONFIG_STDIO_ELEMENT_IDX_STDOUT] = {
        .name = "stdout",
        .data = &config_stdio_element_cfg[CONFIG_STDIO_ELEMENT_IDX_STDOUT],
    },

    [CONFIG_STDIO_ELEMENT_IDX_COUNT] = { 0 },
};

static FWK_CONSTRUCTOR void config_stdio_init(void)
{
    config_stdio_element_cfg[CONFIG_STDIO_ELEMENT_IDX_STDIN].stream = stdin;
    config_stdio_element_cfg[CONFIG_STDIO_ELEMENT_IDX_STDOUT].stream = stdout;
}

const struct fwk_module_config config_stdio = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(config_stdio_elements),
};
