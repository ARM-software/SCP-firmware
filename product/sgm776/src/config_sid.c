/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm776_mmap.h"
#include "sgm776_sid.h"

#include <mod_sid.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element subsystem_table[] = {
    {
        .name = "Duke",
        .data = &(struct mod_sid_subsystem_config) {
            .part_number = SGM776_SID_PART_DUKE,
        }
    },
    {
        .name = "Davis",
        .data = &(struct mod_sid_subsystem_config) {
            .part_number = SGM776_SID_PART_DAVIS,
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
        .sid_base = SID_BASE,
        .valid_pcid_registers = MOD_PCID_REGISTER_ALL,
        .pcid_expected = {
            .PID0 = 0xB9,
            .PID1 = 0xB0,
            .PID2 = 0x0B,
            .PID4 = 0x04,
            .CID0 = 0x0D,
            .CID1 = 0xF0,
            .CID2 = 0x05,
            .CID3 = 0xB1,
        },
    },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_subsystem_table),
};
