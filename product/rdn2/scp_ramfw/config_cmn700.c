/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "platform_def.h"
#include "scp_css_mmap.h"
#include "scp_soc_mmap.h"

#include <mod_cmn700.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * CMN700 nodes
 */
#if (PLATFORM_VARIANT == 0)
#    define MEM_CNTRL0_ID 64
#    define MEM_CNTRL1_ID 128
#    define MEM_CNTRL2_ID 192
#    define MEM_CNTRL3_ID 256
#    define MEM_CNTRL4_ID 108
#    define MEM_CNTRL5_ID 172
#    define MEM_CNTRL6_ID 234
#    define MEM_CNTRL7_ID 298

#    define NODE_ID_HND  260
#    define NODE_ID_HNI0 0
#    define NODE_ID_HNP0 324
#    define NODE_ID_HNP1 340
#    define NODE_ID_HNP2 348
#    define NODE_ID_HNP3 362
#    define NODE_ID_HNP4 364
#    define NODE_ID_SBSX 196

#    define MESH_SIZE_X 6
#    define MESH_SIZE_Y 6

#elif (PLATFORM_VARIANT == 1)
#    define MEM_CNTRL0_ID 32
#    define MEM_CNTRL1_ID 64

#    define NODE_ID_HND  68
#    define NODE_ID_HNI0 0
#    define NODE_ID_HNP0 2
#    define NODE_ID_HNP1 3
#    define NODE_ID_SBSX 66

#    define MESH_SIZE_X 3
#    define MESH_SIZE_Y 3
#endif

#if (PLATFORM_VARIANT == 0)
static const unsigned int snf_table[] = {
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
#elif (PLATFORM_VARIANT == 1)
static const unsigned int snf_table[] = {
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 0  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 1  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 2  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 3  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 4  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 5  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 6  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 7  */
};
#endif

static const struct mod_cmn700_mem_region_map mmap[] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x003F_FFFFF_FFFF (4 TB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(4) * FWK_TIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_SYSCACHE,
    },
    {
        /*
         * Boot region
         * Map: 0x0000_0000_0000 - 0x0000_07FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN700_REGION_TYPE_SYSCACHE_SUB,
        .node_id = NODE_ID_SBSX,
    },
    {
        /*
         * Boot Flash
         * Map: 0x00_0800_0000 - 0x00_0FFF_FFFF (128 MB)
         */
        .base = UINT64_C(0x0008000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNI0,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_1000_0000 - 0x00_1FFF_FFFF (256 MB)
         */
        .base = UINT64_C(0x0010000000),
        .size = UINT64_C(256) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals
         * Map: 0x00_2000_0000 - 0x00_3FFF_FFFF (512 MB)
         */
        .base = UINT64_C(0x0020000000),
        .size = UINT64_C(512) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 0
         * Map: 0x00_4000_0000 - 0x00_41FF_FFFF (32 MB)
         */
        .base = UINT64_C(0x0040000000),
        .size = UINT64_C(32) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP0,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 1
         * Map: 0x00_4200_0000 - 0x00_43FF_FFFF (32 MB)
         */
        .base = UINT64_C(0x0042000000),
        .size = UINT64_C(32) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
#if (PLATFORM_VARIANT == 0)
    {
        /*
         * Peripherals, NCI GPV Memory Map 2
         * Map: 0x00_4400_0000 - 0x00_45FF_FFFF (32 MB)
         */
        .base = UINT64_C(0x0044000000),
        .size = UINT64_C(32) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP2,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 3
         * Map: 0x00_4600_0000 - 0x00_47FF_FFFF (32 MB)
         */
        .base = UINT64_C(0x0046000000),
        .size = UINT64_C(32) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP3,
    },
    {
        /*
         * Peripherals, NCI GPV Memory Map 3
         * Map: 0x00_4800_0000 - 0x00_49FF_FFFF (32 MB)
         */
        .base = UINT64_C(0x0048000000),
        .size = UINT64_C(32) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP4,
    },
#endif
    {
        /*
         * Peripherals, PCIe 32-bit MMIO to IO Macro 0
         */
        .base = AP_PCIE_MMIOL_BASE,
        .size = AP_PCIE_MMIOL_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP0,
    },
    {
        /*
         * Peripherals, PCIe 32-bit MMIO to IO Macro 1
         */
        .base = AP_PCIE_MMIOL_BASE + (1 * AP_PCIE_MMIOL_SIZE_PER_RC),
        .size = AP_PCIE_MMIOL_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
#if (PLATFORM_VARIANT == 0)
    {
        /*
         * Peripherals, PCIe 32-bit MMIO to IO Macro 2
         */
        .base = AP_PCIE_MMIOL_BASE + (2 * AP_PCIE_MMIOL_SIZE_PER_RC),
        .size = AP_PCIE_MMIOL_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP2,
    },
    {
        /*
         * Peripherals, PCIe 32-bit MMIO to IO Macro 3
         */
        .base = AP_PCIE_MMIOL_BASE + (3 * AP_PCIE_MMIOL_SIZE_PER_RC),
        .size = AP_PCIE_MMIOL_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP3,
    },
#endif
    {
        /*
         * PCIe ECAM0 to IO Macro 0
         */
        .base = AP_PCIE_ECAM_BASE,
        .size = AP_PCIE_ECAM_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP0,
    },
    {
        /*
         * PCIe ECAM0 to IO Macro 1
         */
        .base = AP_PCIE_ECAM_BASE + (1 * AP_PCIE_ECAM_SIZE_PER_RC),
        .size = AP_PCIE_ECAM_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
#if (PLATFORM_VARIANT == 0)
    {
        /*
         * PCIe ECAM0 to IO Macro 2
         */
        .base = AP_PCIE_ECAM_BASE + (2 * AP_PCIE_ECAM_SIZE_PER_RC),
        .size = AP_PCIE_ECAM_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP2,
    },
    {
        /*
         * PCIe ECAM0 to IO Macro 3
         */
        .base = AP_PCIE_ECAM_BASE + (3 * AP_PCIE_ECAM_SIZE_PER_RC),
        .size = AP_PCIE_ECAM_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP3,
    },
#endif
    {
        /*
         * Peripherals, PCIe 64-bit MMIO to IO Macro 0
         */
        .base = AP_PCIE_MMIOH_BASE,
        .size = AP_PCIE_MMIOH_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP0,
    },
    {
        /*
         * Peripherals, PCIe 64-bit MMIO to IO Macro 1
         */
        .base = AP_PCIE_MMIOH_BASE + (1 * AP_PCIE_MMIOH_SIZE_PER_RC),
        .size = AP_PCIE_MMIOH_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
#if (PLATFORM_VARIANT == 0)
    {
        /*
         * Peripherals, PCIe 64-bit MMIO to IO Macro 2
         */
        .base = AP_PCIE_MMIOH_BASE + (2 * AP_PCIE_MMIOH_SIZE_PER_RC),
        .size = AP_PCIE_MMIOH_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP2,
    },
    {
        /*
         * Peripherals, PCIe 64-bit MMIO to IO Macro 3
         */
        .base = AP_PCIE_MMIOH_BASE + (3 * AP_PCIE_MMIOH_SIZE_PER_RC),
        .size = AP_PCIE_MMIOH_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP3,
    },
#endif
    {
        /*
         * Non Secure NOR Flash 0/1
         * Map: 0x10_5000_0000 - 0x10_57FF_FFFF (128 MB)
         */
        .base = UINT64_C(0x1050000000),
        .size = UINT64_C(128) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
    {
        /*
         * Ethernet Controller PL91x
         * Map: 0x10_5C00_0000 - 0x10_5FFF_FFFF (64 MB)
         */
        .base = UINT64_C(0x105C000000),
        .size = UINT64_C(64) * FWK_MIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
};

static const struct fwk_element cmn700_device_table[] = {
    [0] = { .name = "CMN700 Mesh Config",
            .data = &((struct mod_cmn700_config){
                .base = SCP_CMN700_BASE,
                .mesh_size_x = MESH_SIZE_X,
                .mesh_size_y = MESH_SIZE_Y,
                .hnd_node_id = NODE_ID_HND,
                .snf_table = snf_table,
                .snf_count = FWK_ARRAY_SIZE(snf_table),
                .mmap_table = mmap,
                .mmap_count = FWK_ARRAY_SIZE(mmap),
                .chip_addr_space = UINT64_C(4) * FWK_TIB,
                .clock_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
                .hnf_cal_mode = true,
            }) },
    [1] = { 0 }
};

static const struct fwk_element *cmn700_get_device_table(fwk_id_t module_id)
{
    return cmn700_device_table;
}

const struct fwk_module_config config_cmn700 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(cmn700_get_device_table),
};
