/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_css_mmap.h"

#include <mod_cmn650.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * CMN650 nodes
 */
#define MEM_CNTRL0_ID 332
#define MEM_CNTRL1_ID 333
#define MEM_CNTRL2_ID 16
#define MEM_CNTRL3_ID 17
#define MEM_CNTRL4_ID 32
#define MEM_CNTRL5_ID 33
#define MEM_CNTRL6_ID 348
#define MEM_CNTRL7_ID 349
#define NODE_ID_HND   4
#define NODE_ID_SBSX  340

/*
 * CCIX Related macros
 */
#define CHIP_0      0
#define CHIP_1      1
#define CHIP_2      2
#define CHIP_3      3
#define CCIX_PORT_0 0
#define CCIX_PORT_1 1
#define CCIX_PORT_2 2
#define CCIX_PORT_3 3
#define CCIX_PORT_4 4
#define CCIX_PORT_5 5

#define RNF_PER_CHIP 16
#define CHIP_COUNT   4

static const unsigned int snf_table_0_2[] = {
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 0 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 1 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 2 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 3 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 4 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 5 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 6 */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 7 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 8 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 9 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 10 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 11 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 12 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 13 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 14 */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 15 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 16 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 17 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 18 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 19 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 20 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 21 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 22 */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 23 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 24 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 25 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 26 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 27 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 28 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 29 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 30 */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 31 */
};

static const unsigned int snf_table_1_3[] = {
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 0 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 1 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 2 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 3 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 4 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 5 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 6 */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 7 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 8 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 9 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 10 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 11 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 12 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 13 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 14 */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 15 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 16 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 17 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 18 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 19 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 20 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 21 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 22 */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 23 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 24 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 25 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 26 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 27 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 28 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 29 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 30 */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 31 */
};

static const struct mod_cmn650_mem_region_map mmap[] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x00FF_FFFFF_FFFF (16 TB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(16) * FWK_TIB,
        .type = MOD_CMN650_MEM_REGION_TYPE_SYSCACHE,
    },
    {
        /*
         * Boot region
         * Map: 0x0000_0000_0000 - 0x0000_0001_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN650_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * NOR Flash
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN650_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_1FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN650_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_2000_0000 - 0x00_3FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x0020000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN650_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_4000_0000 - 0x00_7FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x0040000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN650_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * 64-bit PCIe MMIO region
         * Map: 0x05_0000_0000 - 0x05_FFFF_FFFF (4 GB)
         */
        .base = UINT64_C(0x0500000000),
        .size = UINT64_C(4) * FWK_GIB,
        .type = MOD_CMN650_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
};

/* CCIX Related configuration data */

/* Chip-0 Config data */
static const struct mod_cmn650_ccix_config ccix_config_table_chip_0[] = {
    {
        .ldid = CCIX_PORT_0,
        .haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_0,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x40000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x40000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_5,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_2),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_2) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_1,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_1,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_1) + CCIX_PORT_1}
    },
    {
        .ldid = CCIX_PORT_2,
        .haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_2,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x80000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x80000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_3,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_3),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_3) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_3,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_3,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_2) + CCIX_PORT_1}
    },
    {
        .ldid = CCIX_PORT_4,
        .haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_4,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0xC0000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0xC0000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_1),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_1) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_5,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_5,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_3) + CCIX_PORT_1}
    }
};

/* Chip-1 Config data */
static const struct mod_cmn650_ccix_config ccix_config_table_chip_1[] = {
    {
        .ldid = CCIX_PORT_0,
        .haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_0,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x00000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_5,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_0),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_0) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_1,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_1,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_0) + CCIX_PORT_1}
    },
    {
        .ldid = CCIX_PORT_2,
        .haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_2,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x80000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x80000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_3,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_2),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_2) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_3,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_3,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_2) + CCIX_PORT_3}
    },
    {
        .ldid = CCIX_PORT_4,
        .haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_4,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0xC0000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0xC0000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_3),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_3) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_5,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_5,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_3) + CCIX_PORT_3}
    },
};

/* Chip-2 Config data */
static const struct mod_cmn650_ccix_config ccix_config_table_chip_2[] = {
    {
        .ldid = CCIX_PORT_0,
        .haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_0,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x00000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_5,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_3),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_3) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_1,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_1,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_0) + CCIX_PORT_3}
    },
    {
        .ldid = CCIX_PORT_2,
        .haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_2,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x40000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x40000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_3,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_1),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_1) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_3,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_3,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_1) + CCIX_PORT_3}
    },
    {
        .ldid = CCIX_PORT_4,
        .haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_4,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0xC0000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0xC0000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_0),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_0) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_5,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_5,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_3) + CCIX_PORT_5}
    },
};

/* Chip-3 Config data */
static const struct mod_cmn650_ccix_config ccix_config_table_chip_3[] = {
    {
        .ldid = CCIX_PORT_0,
        .haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_0,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x00000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_1) + CCIX_PORT_5,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_1),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_1) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_1,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_1,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_0) + CCIX_PORT_5}
    },
    {
        .ldid = CCIX_PORT_2,
        .haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_2,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x40000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x40000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_0) + CCIX_PORT_3,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_0),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_0) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_3,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_3,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_1) + CCIX_PORT_5}
    },
    {
        .ldid = CCIX_PORT_4,
        .haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_4,
        .remote_rnf_count = RNF_PER_CHIP * (CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x80000000000),
            .size = UINT64_C(4) * FWK_TIB,
            .type = MOD_CMN650_REGION_TYPE_CCIX,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x80000000000),
                .size = UINT64_C(4) * FWK_TIB,
                .remote_haid = (RNF_PER_CHIP * CHIP_2) + CCIX_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP * CHIP_2),
                .remote_agentid_end = (RNF_PER_CHIP * CHIP_2) + RNF_PER_CHIP - 1
            },
        },
        .smp_mode = true,
        .port_aggregate = true,
        .port_aggregate_ldid = CCIX_PORT_5,
        .port_aggregate_haid = (RNF_PER_CHIP * CHIP_3) + CCIX_PORT_5,
        .port_aggregate_remote_haid = {(RNF_PER_CHIP * CHIP_2) + CCIX_PORT_5}
    },
};

static const struct fwk_element cmn650_device_table[] = {
    [0] = {
        .name = "Chip-0 CMN-650 Mesh Config",
        .data = &((struct mod_cmn650_config){
                .base = SCP_CMN650_BASE,
                .mesh_size_x = 6,
                .mesh_size_y = 6,
                .hnd_node_id = NODE_ID_HND,
                .snf_table = snf_table_0_2,
                .snf_count = FWK_ARRAY_SIZE(snf_table_0_2),
                .mmap_table = mmap,
                .mmap_count = FWK_ARRAY_SIZE(mmap),
                .ccix_config_table = ccix_config_table_chip_0,
                .ccix_table_count = FWK_ARRAY_SIZE(ccix_config_table_chip_0),
                .chip_addr_space = UINT64_C(4) * FWK_TIB,
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
                .hnf_cal_mode = true,
            }),
    },
    [1] = {
        .name = "Chip-1 CMN-650 Mesh Config",
        .data = &((struct mod_cmn650_config){
                .base = SCP_CMN650_BASE,
                .mesh_size_x = 6,
                .mesh_size_y = 6,
                .hnd_node_id = NODE_ID_HND,
                .snf_table = snf_table_1_3,
                .snf_count = FWK_ARRAY_SIZE(snf_table_1_3),
                .mmap_table = mmap,
                .mmap_count = FWK_ARRAY_SIZE(mmap),
                .ccix_config_table = ccix_config_table_chip_1,
                .ccix_table_count = FWK_ARRAY_SIZE(ccix_config_table_chip_1),
                .chip_addr_space = UINT64_C(4) * FWK_TIB,
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
                .hnf_cal_mode = true,
            }),
    },
    [2] = {
        .name = "Chip-2 CMN-650 Mesh Config",
        .data = &((struct mod_cmn650_config){
                .base = SCP_CMN650_BASE,
                .mesh_size_x = 6,
                .mesh_size_y = 6,
                .hnd_node_id = NODE_ID_HND,
                .snf_table = snf_table_0_2,
                .snf_count = FWK_ARRAY_SIZE(snf_table_0_2),
                .mmap_table = mmap,
                .mmap_count = FWK_ARRAY_SIZE(mmap),
                .ccix_config_table = ccix_config_table_chip_2,
                .ccix_table_count = FWK_ARRAY_SIZE(ccix_config_table_chip_2),
                .chip_addr_space = UINT64_C(4) * FWK_TIB,
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
                .hnf_cal_mode = true,
            }),
    },
    [3] = {
        .name = "Chip-3 CMN-650 Mesh Config",
        .data = &((struct mod_cmn650_config){
                .base = SCP_CMN650_BASE,
                .mesh_size_x = 6,
                .mesh_size_y = 6,
                .hnd_node_id = NODE_ID_HND,
                .snf_table = snf_table_1_3,
                .snf_count = FWK_ARRAY_SIZE(snf_table_1_3),
                .mmap_table = mmap,
                .mmap_count = FWK_ARRAY_SIZE(mmap),
                .ccix_config_table = ccix_config_table_chip_3,
                .ccix_table_count = FWK_ARRAY_SIZE(ccix_config_table_chip_3),
                .chip_addr_space = UINT64_C(4) * FWK_TIB,
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
                .hnf_cal_mode = true,
            }),
    },
    [4] = { 0 }
};

static const struct fwk_element *cmn650_get_device_table(fwk_id_t module_id)
{
    return cmn650_device_table;
}

const struct fwk_module_config config_cmn650 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(cmn650_get_device_table),
};
