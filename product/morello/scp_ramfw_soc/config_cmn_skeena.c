/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "morello_scp_mmap.h"

#include <mod_cmn_skeena.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * CMN_SKEENA nodes
 */
#define DMC0_ID      268
#define DMC1_ID      260
#define NODE_ID_HND  68
#define NODE_ID_HNI  72
#define NODE_ID_SBSX 128
#define NODE_ID_CCIX 76

static const unsigned int snf_table[4] = {
    DMC0_ID, /* Maps to HN-F logical node 0 */
    DMC0_ID, /* Maps to HN-F logical node 1 */
    DMC1_ID, /* Maps to HN-F logical node 2 */
    DMC1_ID, /* Maps to HN-F logical node 3 */
};

static const unsigned int rni_pcie_list[4] = { 12, 8, 4, 0 };

static const struct mod_cmn_skeena_memory_region_map mmap[19] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x07FF_FFFF_FFFF (8 TB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(8) * FWK_TIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_SYSCACHE,
    },
    {
        /*
         * Boot region
         * Map: 0x0000_0000_0000 - 0x0000_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_SKEENA_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_1FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_2000_0000 - 0x00_3FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x0020000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_4000_0000 - 0x00_5FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x0040000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_6000_0000 - 0x00_6FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0060000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_7000_0000 - 0x00_7FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0070000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI,
    },
    {
        /*
         * Peripherals
         * Map: 0x04_0000_0000 - 0x07_FFFF_FFFF (16 GB)
         */
        .base = UINT64_C(0x0400000000),
        .size = UINT64_C(16) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x08_0000_0000 - 0x0F_FFFF_FFFF (32 GB)
         */
        .base = UINT64_C(0x0800000000),
        .size = UINT64_C(32) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x10_0000_0000 - 0x1F_FFFF_FFFF (64 GB)
         */
        .base = UINT64_C(0x1000000000),
        .size = UINT64_C(64) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x20_0000_0000 - 0x27_FFFF_FFFF (32 GB)
         */
        .base = UINT64_C(0x2000000000),
        .size = UINT64_C(32) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x28_0000_0000 - 0x2F_FFFF_FFFF (32 GB)
         */
        .base = UINT64_C(0x2800000000),
        .size = UINT64_C(32) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x30_0000_0000 - 0x3F_FFFF_FFFF (64 GB)
         */
        .base = UINT64_C(0x3000000000),
        .size = UINT64_C(64) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI,
    },
    {
        /*
         * Peripherals
         * Map: 0x40_0000_0000 - 0x4F_FFFF_FFFF (64 GB)
         */
        .base = UINT64_C(0x4000000000),
        .size = UINT64_C(64) * FWK_GIB,
        .type = MOD_CMN_SKEENA_MEMORY_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI,
    },
    {
        /*
         * Peripherals
         * Map: 0x50_0000_0000 - 0x5F_FFFF_FFFF (64 GB)
         */
        .base = UINT64_C(0x5000000000),
        .size = UINT64_C(64) * FWK_GIB,
        .type = MOD_CMN_SKEENA_REGION_TYPE_SYSCACHE_NONHASH,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x60_0000_0000 - 0x7F_FFFF_FFFF (128 GB)
         */
        .base = UINT64_C(0x6000000000),
        .size = UINT64_C(128) * FWK_GIB,
        .type = MOD_CMN_SKEENA_REGION_TYPE_SYSCACHE_NONHASH,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x80_0000_0000 - 0x80_7FFF_FFFF (512 GB - 514 GB)
         */
        .base = UINT64_C(0x8000000000),
        .size = UINT64_C(2) * FWK_GIB,
        .type = MOD_CMN_SKEENA_REGION_TYPE_SYSCACHE_NONHASH,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x400_0000_0000 - 0x7FF_FFFF_FFFF (4 TB)
         */
        .base = UINT64_C(0x40000000000),
        .size = UINT64_C(4) * FWK_TIB,
        .type = MOD_CMN_SKEENA_REGION_TYPE_CCIX,
        .node_id = NODE_ID_CCIX,
    },
};

const struct fwk_module_config config_cmn_skeena = {
    .data =
        &(struct mod_cmn_skeena_config){
            .base = SCP_CMN_SKEENA_BASE,
            .mesh_size_x = 5,
            .mesh_size_y = 2,
            .hnd_node_id = NODE_ID_HND,
            .snf_table = snf_table,
            .snf_count = FWK_ARRAY_SIZE(snf_table),
            .sa_count = 2,
            .mmap_table = mmap,
            .mmap_count = FWK_ARRAY_SIZE(mmap),
            .rni_pcie_table = rni_pcie_list,
            .rni_pcie_count = FWK_ARRAY_SIZE(rni_pcie_list),
            .chip_addr_space = UINT64_C(4) * FWK_TIB,
            .clock_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_CLOCK,
                CLOCK_IDX_INTERCONNECT),
            .hnf_cal_mode = false,
        },
};
