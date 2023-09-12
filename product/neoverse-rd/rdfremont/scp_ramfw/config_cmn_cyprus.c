/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'cmn_cyprus'.
 */

#include "cmn_node_id.h"
#include "platform_core.h"
#include "scp_css_mmap.h"
#include "scp_exp_mmap.h"

#include <mod_cmn_cyprus.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Number of entries in the 'snf_table' */
#define SNF_TABLE_COUNT 32

/* Number of entries in the 'mmap_table' */
#define MMAP_TABLE_COUNT 17

static const unsigned int snf_table[SNF_TABLE_COUNT] = {
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 0  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 1  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 2  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 3  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 4  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 5  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 6  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 7  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 8  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 9  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 10  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 11  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 12  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 13  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 14  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 15  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 16  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 17  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 18  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 19  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 20  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 21  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 22  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 23  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 24  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 25  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 26  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 27  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 28  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 29  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 30  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 31  */
};

static const struct mod_cmn_cyprus_mem_region_map mmap[MMAP_TABLE_COUNT] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x03FF_FFFF_FFFF (4 TiB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(256) * FWK_TIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE,
        .hns_pos_start = { 0, 0, 0 },
        .hns_pos_end = { MESH_SIZE_X - 1, MESH_SIZE_Y - 1, 1 },
    },
    {
        /*
         * Shared SRAM
         * Map: 0x0000_0000_0000 - 0x0000_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Boot Flash
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNT1,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_2EFF_FFFF (496 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(496) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Shared SRAM
         * Map: 0x00_2F00_0000 - 0x00_2F3F_FFFF (4 MB)
         */
        .base = UINT64_C(0x002F000000),
        .size = UINT64_C(4) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_2F40_0000 - 0x00_5FFF_FFFF (780 MB)
         */
        .base = UINT64_C(0x002F400000),
        .size = UINT64_C(780) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * CMN_CYPRUS GPV
         * Map: 0x01_0000_0000 - 0x01_3FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x0100000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Cluster Utility Memory region
         * Map: 0x2_0000_0000 - 0x2_3FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x200000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals - Memory Controller
         * Map: 0x2_4000_0000 - 0x2_4FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x240000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 0
         * Map: 0x02_8000_0000 - 0x02_87FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0280000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID0,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 1
         * Map: 0x02_8800_0000 - 0x02_8FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0288000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID1,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 2
         * Map: 0x02_9000_0000 - 0x02_97FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0290000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID2,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 3
         * Map: 0x02_9800_0000 - 0x02_9FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0298000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID3,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 4
         * Map: 0x02_A000_0000 - 0x02_A7FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x02A0000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = IOVB_NODE_ID4,
    },
    {
        /*
         * GPC_SMMU region
         * Map: 0x03_0000_0000 - 0x03_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x300000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Non Secure NOR Flash 0/1
         * Map: 0x06_0000_0000 - 0x06_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0600000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Ethernet Controller PL91x
         * Map: 0x06_0C00_0000 - 0x06_0FFF_FFFF (64 MB)
         */
        .base = UINT64_C(0x060C000000),
        .size = UINT64_C(64) * FWK_MIB,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
};

static struct mod_cmn_cyprus_config cmn_config_data = {
    .periphbase = SCP_CMN_BASE,
    .mesh_size_x = MESH_SIZE_X,
    .mesh_size_y = MESH_SIZE_Y,
    .mmap_table = mmap,
    .mmap_count = FWK_ARRAY_SIZE(mmap),
    .hns_cal_mode = true,
    .hnf_sam_config = {
        .snf_table = snf_table,
        .snf_count = FWK_ARRAY_SIZE(snf_table),
        .hnf_sam_mode = MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING,
    },
    .rnsam_scg_config = {
        .scg_hashing_mode = MOD_CMN_CYPRUS_RNSAM_SCG_POWER_OF_TWO_HASHING,
    },
};

const struct fwk_module_config config_cmn_cyprus = {
    .data = (void *)&cmn_config_data,
};
