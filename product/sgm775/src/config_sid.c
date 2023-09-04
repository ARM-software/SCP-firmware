/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm775_mmap.h"
#include "sgm775_ssc.h"

#include <mod_sid.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element subsystem_table[] = {
    {
        .name = "SGM-775",
        .data = &(struct mod_sid_subsystem_config) {
            .part_number = SSC_PART_SGM_775,
        }
    },
    { 0 },
};

static const struct fwk_element *get_subsystem_table(fwk_id_t id)
{
    return subsystem_table;
}

struct fwk_module_config config_sid = {
    .data = &(struct mod_sid_config) {
        .sid_base = SSC_BASE,
        .valid_pcid_registers = MOD_PCID_REGISTER_ALL,
        .pcid_expected = {
            .PID0 = 0x44,
            .PID1 = 0xB8,
            .PID2 = 0x1B,
            .PID4 = 0x04,
            .CID0 = 0x0D,
            .CID1 = 0xF0,
            .CID2 = 0x05,
            .CID3 = 0xB1,
        },
    },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_subsystem_table),
};
