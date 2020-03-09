/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "scp_rdn1e1_mmap.h"

#include <mod_cmn600.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROOT_NODE_OFFSET 0xD00000

/*
 * CMN600 nodes
 */
#define DMC0_ID 0x60
#define DMC1_ID 0x6c
#define NODE_ID_HND 0x68
#define NODE_ID_SBSX 0x64
#define NODE_ID_CCIX 0x0

static const unsigned int snf_table[] = {
    DMC0_ID, /* Maps to HN-F logical node 0 */
    DMC0_ID, /* Maps to HN-F logical node 1 */
    DMC1_ID, /* Maps to HN-F logical node 2 */
    DMC1_ID, /* Maps to HN-F logical node 3 */
};

static const struct mod_cmn600_memory_region_map mmap[] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x07FF_FFFF_FFFF (8 TB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(8) * FWK_TIB,
        .type = MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE,
    },
    {
        /*
         * Boot region
         * Map: 0x0000_0000_0000 - 0x0000_0001_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN600_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN600_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_1FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN600_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_2000_0000 - 0x00_3FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x0020000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN600_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_4000_0000 - 0x00_7FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x0040000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN600_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Remote chip address space
         * Map: 0x400_0000_0000 - 0x7FF_FFFF_FFFF (4 TB to 8 TB)
         */
        .base = UINT64_C(0x40000000000),
        .size = UINT64_C(4) * FWK_TIB,
        .type = MOD_CMN600_REGION_TYPE_CCIX,
        .node_id = NODE_ID_CCIX,
    },
};

const struct fwk_module_config config_cmn600 = {
    .get_element_table = NULL,
    .data = &((struct mod_cmn600_config) {
        .base = SCP_CMN600_BASE,
        .mesh_size_x = 4,
        .mesh_size_y = 2,
        .hnd_node_id = NODE_ID_HND,
        .snf_table = snf_table,
        .snf_count = FWK_ARRAY_SIZE(snf_table),
        .mmap_table = mmap,
        .mmap_count = FWK_ARRAY_SIZE(mmap),
        .chip_addr_space = UINT64_C(4) * FWK_TIB,
        .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
            CLOCK_IDX_INTERCONNECT),
        .hnf_cal_mode = false,
    }),
};
