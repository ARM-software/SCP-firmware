/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "platform_core.h"
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

/* CCG ports available on the variant 2. */
enum rdn2cfg2_cmn700_ccg_port {
    CCG_PORT_0,
    CCG_PORT_1,
    CCG_PORT_2,
    CCG_PORT_3,
    CCG_PORT_4,
    CCG_PER_CHIP,
};

/* Total RN-Fs (N2 CPUs) per chips for variant 2. */
#define RNF_PER_CHIP_CFG2 4

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

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP4

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

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP1

#    define MESH_SIZE_X 3
#    define MESH_SIZE_Y 3

#elif (PLATFORM_VARIANT == 2)
#    define MEM_CNTRL0_ID 108
#    define MEM_CNTRL1_ID 172
#    define MEM_CNTRL2_ID 234
#    define MEM_CNTRL3_ID 298

#    define NODE_ID_HND  256
#    define NODE_ID_HNI0 0
#    define NODE_ID_HNP0 0x144
#    define NODE_ID_HNP1 0x154
#    define NODE_ID_HNP2 0x15c
#    define NODE_ID_HNP3 0x16a
#    define NODE_ID_SBSX 196

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP1

#    define MESH_SIZE_X 6
#    define MESH_SIZE_Y 6

#elif (PLATFORM_VARIANT == 3)
#    define MEM_CNTRL0_ID 16
#    define MEM_CNTRL1_ID 1168
#    define MEM_CNTRL2_ID 8
#    define MEM_CNTRL3_ID 1160
#    define MEM_CNTRL4_ID 24
#    define MEM_CNTRL5_ID 1176
#    define MEM_CNTRL6_ID 32
#    define MEM_CNTRL7_ID 1184

#    define NODE_ID_HND  42
#    define NODE_ID_HNI0 256
#    define NODE_ID_HNP0 44
#    define NODE_ID_HNP1 298
#    define NODE_ID_HNP2 554
#    define NODE_ID_HNP3 682
#    define NODE_ID_HNP4 938
#    define NODE_ID_SBSX 172

#    define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP4

#    define MESH_SIZE_X 10
#    define MESH_SIZE_Y 6
#endif

#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
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
#    if (PLATFORM_VARIANT == 3)
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 32  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 33  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 34  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 35  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 36  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 37  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 38  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 39  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 40  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 41  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 42  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 43  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 44  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 45  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 46  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 47  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 48  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 49  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 50  */
    MEM_CNTRL4_ID, /* Maps to HN-F logical node 51  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 52  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 53  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 54  */
    MEM_CNTRL5_ID, /* Maps to HN-F logical node 55  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 56  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 57  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 58  */
    MEM_CNTRL6_ID, /* Maps to HN-F logical node 59  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 60  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 61  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 62  */
    MEM_CNTRL7_ID, /* Maps to HN-F logical node 63  */
#    endif
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
#elif (PLATFORM_VARIANT == 2)
static const unsigned int snf_table[] = {
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 0  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 1  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 2  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 3  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 4  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 5  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 6  */
    MEM_CNTRL0_ID, /* Maps to HN-F logical node 7  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 8  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 9  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 10  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 11  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 12  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 13  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 14  */
    MEM_CNTRL1_ID, /* Maps to HN-F logical node 15  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 16  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 17  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 18  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 19  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 20  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 21  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 22  */
    MEM_CNTRL2_ID, /* Maps to HN-F logical node 23  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 24  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 25  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 26  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 27  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 28  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 29  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 30  */
    MEM_CNTRL3_ID, /* Maps to HN-F logical node 31  */
};
#endif

static const struct mod_cmn700_mem_region_map mmap[] = {
    {
        /*
         * System cache backed region
         * Map: 0x0000_0000_0000 - 0x3FFF_FFFF_FFFF (64 TiB)
         */
        .base = UINT64_C(0x000000000000),
        .size = UINT64_C(64) * FWK_TIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_SYSCACHE,
        .hnf_pos_start = { 0, 0, 0 },
        .hnf_pos_end = { MESH_SIZE_X - 1, MESH_SIZE_Y - 1, 1 },
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
#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
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
         * Peripherals, NCI GPV Memory Map 4
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
#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
    {
        /*
         * Peripherals, PCIe 32-bit MMIO to IO Macro 1
         */
        .base = AP_PCIE_MMIOL_BASE + (1 * AP_PCIE_MMIOL_SIZE_PER_RC),
        .size = AP_PCIE_MMIOL_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
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
#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
    {
        /*
         * PCIe ECAM0 to IO Macro 1
         */
        .base = AP_PCIE_ECAM_BASE + (1 * AP_PCIE_ECAM_SIZE_PER_RC),
        .size = AP_PCIE_ECAM_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
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
#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
    {
        /*
         * Peripherals, PCIe 64-bit MMIO to IO Macro 1
         */
        .base = AP_PCIE_MMIOH_BASE + (1 * AP_PCIE_MMIOH_SIZE_PER_RC),
        .size = AP_PCIE_MMIOH_SIZE_PER_RC,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HNP1,
    },
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
         * CMN700 GPV
         * Map: 0x01_4000_0000 - 0x01_7FFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x0140000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_HND,
    },
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
    {
        /*
         * Non-PCIe IO Macro
         * Map: 0x10_8000_0000 - 0x10_BFFF_FFFF (1 GB)
         */
        .base = UINT64_C(0x1080000000),
        .size = UINT64_C(1) * FWK_GIB,
        .type = MOD_CMN700_MEM_REGION_TYPE_IO,
        .node_id = NODE_ID_NON_PCIE_IO_MACRO,
    },
};

#if (PLATFORM_VARIANT == 2)
/* Multichip Related configuration data */

/*
 * Cross chip CCG connections between the chips:
 *
 *   +-------------------------------+      +-------------------------------+
 *   |                               |      |                               |
 *   |                               |      |                               |
 *   |          Chip 0               |      |          Chip 2               |
 *   |                               |      |                               |
 *   |                               |      |                               |
 *   |  CCG4  CCG3  CCG2  CCG1  CCG0 |      |  CCG4  CCG3  CCG2  CCG1  CCG0 |
 *   +---------+-----+-------+---+---+      +---+-----+-----+-------+-------+
 *             |     |       |   |              |     |     |       |
 *             |     +----+  |   +--------------+     |     +-----+ |
 *             |          |  |                        |           | +---------+
 *             +-----+    |  |                        +-----+     |           |
 *                   |    |  |                              |     |           |
 *                   |    |  +--------------------------+   |     |           |
 *                   |    |                             |   |     |           |
 *    +--------------+----+-----------+       +---------+---+-----+---------+ |
 *    | CCG0  CCG1  CCG2  CCG3  CCG4  |       |CCG0  CCG1  CCG2  CCG3  CCG4 | |
 *    |         |                |    |       |  |                          | |
 *    |         |                |    |       |  |                          | |
 *    |         |    Chip 1      +----+-------+--+        Chip 3            | |
 *    |         |                     |       |                             | |
 *    |         +---------------------+-------+-----------------------------+-+
 *    |                               |       |                             |
 *    +-------------------------------+       +-----------------------------+
 */

/* Chip-0 Config data */
static const struct mod_cmn700_ccg_config ccg_config_table_chip_0[] = {
    {
        .ldid = CCG_PORT_3,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_0) + CCG_PORT_3,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x400000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x400000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_1) + CCG_PORT_2,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_1),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_1) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_0,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_0) + CCG_PORT_0,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x800000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x800000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_2) + CCG_PORT_4,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_2),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_2) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_1,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_0) + CCG_PORT_1,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0xC00000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0xC00000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_3) + CCG_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_3),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_3) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
};

/* Chip-1 Config data */
static const struct mod_cmn700_ccg_config ccg_config_table_chip_1[] = {
    {
        .ldid = CCG_PORT_2,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_1) + CCG_PORT_2,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x00000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_0) + CCG_PORT_3,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_0),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_0) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_1,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_1) + CCG_PORT_1,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x800000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x800000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_2) + CCG_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_2),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_2) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_4,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_1) + CCG_PORT_4,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0xC00000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0xC00000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_3) + CCG_PORT_0,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_3),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_3) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
};

/* Chip-2 Config data */
static const struct mod_cmn700_ccg_config ccg_config_table_chip_2[] = {
    {
        .ldid = CCG_PORT_4,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_2) + CCG_PORT_4,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x00000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_0) + CCG_PORT_0,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_0),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_0) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_1,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_2) + CCG_PORT_1,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x400000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x400000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_1) + CCG_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_1),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_1) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_3,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_2) + CCG_PORT_3,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0xC00000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0xC00000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_3) + CCG_PORT_2,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_3),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_3) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
};

/* Chip-3 Config data */
static const struct mod_cmn700_ccg_config ccg_config_table_chip_3[] = {
    {
        .ldid = CCG_PORT_1,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_3) + CCG_PORT_1,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x00000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_0) + CCG_PORT_1,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_0),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_0) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_0,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_3) + CCG_PORT_0,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x400000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x400000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_1) + CCG_PORT_4,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_1),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_1) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
    {
        .ldid = CCG_PORT_2,
        .haid = (CCG_PER_CHIP * PLATFORM_CHIP_3) + CCG_PORT_2,
        .remote_rnf_count = RNF_PER_CHIP_CFG2 * (PLATFORM_CHIP_COUNT - 1),
        .remote_mmap_table = {
            .base = UINT64_C(0x800000000000),
            .size = UINT64_C(64) * FWK_TIB,
            .type = MOD_CMN700_REGION_TYPE_CCG,
        },
        .ra_mmap_table = {
            {
                .base = UINT64_C(0x800000000000),
                .size = UINT64_C(64) * FWK_TIB,
                .remote_haid = (CCG_PER_CHIP * PLATFORM_CHIP_2) + CCG_PORT_3,
            },
            { 0 }
        },
        .remote_agentid_to_linkid_map = {
            {
                .remote_agentid_start = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_2),
                .remote_agentid_end = (RNF_PER_CHIP_CFG2 * PLATFORM_CHIP_2) +
                    RNF_PER_CHIP_CFG2 - 1
            },
        },
        .smp_mode = true,
        .ull_to_ull_mode = true,
    },
};
#endif

static const struct fwk_element cmn700_device_table[] = {
    [PLATFORM_CHIP_0] = { .name = "Chip-0 CMN700 Mesh Config",
                          .data = &((struct mod_cmn700_config) {
                              .base = SCP_CMN700_BASE,
                              .mesh_size_x = MESH_SIZE_X,
                              .mesh_size_y = MESH_SIZE_Y,
                              .hnd_node_id = NODE_ID_HND,
                              .snf_table = snf_table,
                              .snf_count = FWK_ARRAY_SIZE(snf_table),
                              .mmap_table = mmap,
                              .mmap_count = FWK_ARRAY_SIZE(mmap),
#if (PLATFORM_VARIANT == 2)
                              .ccg_config_table = ccg_config_table_chip_0,
                              .ccg_table_count =
                                  FWK_ARRAY_SIZE(ccg_config_table_chip_0),
#endif
                              .chip_addr_space = UINT64_C(64) * FWK_TIB,
                              .clock_id = FWK_ID_ELEMENT_INIT(
                                  FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
                              .hnf_cal_mode = true,
                          }) },
#if (PLATFORM_VARIANT == 2)
    [PLATFORM_CHIP_1] = { .name = "Chip-1 CMN700 Mesh Config",
                          .data = &((struct mod_cmn700_config){
                              .base = SCP_CMN700_BASE,
                              .mesh_size_x = MESH_SIZE_X,
                              .mesh_size_y = MESH_SIZE_Y,
                              .hnd_node_id = NODE_ID_HND,
                              .snf_table = snf_table,
                              .snf_count = FWK_ARRAY_SIZE(snf_table),
                              .mmap_table = mmap,
                              .mmap_count = FWK_ARRAY_SIZE(mmap),
                              .ccg_config_table = ccg_config_table_chip_1,
                              .ccg_table_count =
                                  FWK_ARRAY_SIZE(ccg_config_table_chip_1),
                              .chip_addr_space = UINT64_C(64) * FWK_TIB,
                              .clock_id = FWK_ID_ELEMENT_INIT(
                                  FWK_MODULE_IDX_CLOCK,
                                  CLOCK_IDX_INTERCONNECT),
                              .hnf_cal_mode = true,
                          }) },
    [PLATFORM_CHIP_2] = { .name = "Chip-2 CMN700 Mesh Config",
                          .data = &((struct mod_cmn700_config){
                              .base = SCP_CMN700_BASE,
                              .mesh_size_x = MESH_SIZE_X,
                              .mesh_size_y = MESH_SIZE_Y,
                              .hnd_node_id = NODE_ID_HND,
                              .snf_table = snf_table,
                              .snf_count = FWK_ARRAY_SIZE(snf_table),
                              .mmap_table = mmap,
                              .mmap_count = FWK_ARRAY_SIZE(mmap),
                              .ccg_config_table = ccg_config_table_chip_2,
                              .ccg_table_count =
                                  FWK_ARRAY_SIZE(ccg_config_table_chip_2),
                              .chip_addr_space = UINT64_C(64) * FWK_TIB,
                              .clock_id = FWK_ID_ELEMENT_INIT(
                                  FWK_MODULE_IDX_CLOCK,
                                  CLOCK_IDX_INTERCONNECT),
                              .hnf_cal_mode = true,
                          }) },
    [PLATFORM_CHIP_3] = { .name = "Chip-3 CMN700 Mesh Config",
                          .data = &((struct mod_cmn700_config){
                              .base = SCP_CMN700_BASE,
                              .mesh_size_x = MESH_SIZE_X,
                              .mesh_size_y = MESH_SIZE_Y,
                              .hnd_node_id = NODE_ID_HND,
                              .snf_table = snf_table,
                              .snf_count = FWK_ARRAY_SIZE(snf_table),
                              .mmap_table = mmap,
                              .mmap_count = FWK_ARRAY_SIZE(mmap),
                              .ccg_config_table = ccg_config_table_chip_3,
                              .ccg_table_count =
                                  FWK_ARRAY_SIZE(ccg_config_table_chip_3),
                              .chip_addr_space = UINT64_C(64) * FWK_TIB,
                              .clock_id = FWK_ID_ELEMENT_INIT(
                                  FWK_MODULE_IDX_CLOCK,
                                  CLOCK_IDX_INTERCONNECT),
                              .hnf_cal_mode = true,
                          }) },
#endif
    [PLATFORM_CHIP_COUNT] = { 0 }
};

static const struct fwk_element *cmn700_get_device_table(fwk_id_t module_id)
{
    return cmn700_device_table;
}

const struct fwk_module_config config_cmn700 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(cmn700_get_device_table),
};
