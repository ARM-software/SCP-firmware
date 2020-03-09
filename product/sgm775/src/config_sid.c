/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
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
    .get_element_table = get_subsystem_table,
    .data = &(struct mod_sid_config) {
        .sid_base = SSC_BASE,
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
};
