/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing configuration register.
 */

#include <internal/cmn_cyprus_cfg_reg.h>
#include <internal/cmn_cyprus_reg.h>

#include <stdint.h>

/* Peripheral ID Revision Numbers */
#define PERIPH_ID_2_MASK    UINT64_C(0xFF)
#define PERIPH_ID_2_REV_POS 4

#define CHILD_POINTER_OFFSET UINT64_C(0x000000003FFFFFFF)

unsigned int cfgm_get_cmn_revision(struct cmn_cyprus_cfgm_reg *cfgm)
{
    return ((cfgm->PERIPH_ID[1] & PERIPH_ID_2_MASK) >> PERIPH_ID_2_REV_POS);
}

struct cmn_cyprus_node_cfg_reg *cfgm_get_child_node(
    struct cmn_cyprus_cfgm_reg *cfgm,
    unsigned int child_index)
{
    uintptr_t periphbase;
    unsigned int offset;
    struct cmn_cyprus_node_cfg_reg *child_node;

    offset = (cfgm->CHILD_POINTER[child_index] & CHILD_POINTER_OFFSET);
    /*
     * PERIPHBASE is the starting address of the range that all CMN‑Cyprus
     * configuration registers are mapped.
     */
    periphbase = (uintptr_t)cfgm;

    child_node = (struct cmn_cyprus_node_cfg_reg *)(periphbase + offset);

    return child_node;
}
