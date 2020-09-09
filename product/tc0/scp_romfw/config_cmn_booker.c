/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_css_mmap.h"

#include <mod_cmn_booker.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * CMN_BOOKER nodes
 */
#define MEM_CNTRL0_ID 2
#define MEM_CNTRL1_ID 14
#define MEM_CNTRL2_ID 38
#define MEM_CNTRL3_ID 46
#define NODE_ID_HND 4

static const unsigned int snf_table[] = {
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 0 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 1 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 2 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 3 */
};

static const struct mod_cmn_booker_mem_region_map mmap[] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x003FF_FFFF_FFFF (4 TB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(4) * FWK_TIB,
        .type = MOD_CMN_BOOKER_MEM_REGION_TYPE_SYSCACHE,
    },
    {
        /*
         * Boot region
         * Map: 0x0000_0000_0000 - 0x0000_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_BOOKER_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_BOOKER_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_1FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN_BOOKER_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_2000_0000 - 0x00_3FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x0020000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN_BOOKER_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_4000_0000 - 0x00_7FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x0040000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN_BOOKER_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
};

const struct fwk_module_config config_cmn_booker = {
    .data = &((struct mod_cmn_booker_config){
        .base = SCP_CMN_BOOKER_BASE,
        .mesh_size_x = 2,
        .mesh_size_y = 2,
        .hnd_node_id = NODE_ID_HND,
        .snf_table = snf_table,
        .snf_count = FWK_ARRAY_SIZE(snf_table),
        .mmap_table = mmap,
        .mmap_count = FWK_ARRAY_SIZE(mmap),
        .clock_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        .hnf_cal_mode = false,
    }),
};
