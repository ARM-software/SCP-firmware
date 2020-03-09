/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_rdn1e1_mmap.h"

#include <mod_sid.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element subsystem_table[] = {
    {
        .name = "RDN1E1",
        .data = &(struct mod_sid_subsystem_config) {
            .part_number = 0x786,
        }
    },
    {
        .name = "Christensen",
        .data = &(struct mod_sid_subsystem_config) {
            .part_number = 0x785,
        }
    },
    { 0 },
};

static const struct fwk_element *get_subsystem_table(fwk_id_t id)
{
    return subsystem_table;
}

const struct fwk_module_config config_sid = {
    .get_element_table = get_subsystem_table,
    .data = &(struct mod_sid_config) {
        .sid_base = SCP_SID_BASE,
        .pcid_expected = {
            .PID0 = 0xD2,
            .PID1 = 0xB0,
            .PID2 = 0x0B,
            .PID3 = 0x00,
            .PID4 = 0x04,
            .CID0 = 0x0D,
            .CID1 = 0xF0,
            .CID2 = 0x05,
            .CID3 = 0xB1,
        },
    },
};
