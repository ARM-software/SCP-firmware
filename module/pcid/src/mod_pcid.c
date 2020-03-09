/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_pcid.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <string.h>

bool mod_pcid_check_registers(const struct mod_pcid_registers *registers,
                              const struct mod_pcid_registers *expected)
{
    assert(registers != NULL);
    assert(expected != NULL);

    return !memcmp(registers, expected, sizeof(*registers));
}

static int pcid_init(fwk_id_t module_id,
                     unsigned int element_count,
                     const void *data)
{
    return FWK_SUCCESS;
}

const struct fwk_module_config config_pcid = { 0 };
const struct fwk_module module_pcid = {
    .name = "PCID",
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = pcid_init,
};
