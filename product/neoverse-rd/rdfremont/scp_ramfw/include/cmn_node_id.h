/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CMN node ID.
 */

#ifndef CMN_NODE_ID
#define CMN_NODE_ID

/*
 * CMN node ids
 */
#define MEM_CNTRL0_ID 64
#define MEM_CNTRL1_ID 128
#define MEM_CNTRL2_ID 192
#define MEM_CNTRL3_ID 256
#define MEM_CNTRL4_ID 104
#define MEM_CNTRL5_ID 168
#define MEM_CNTRL6_ID 232
#define MEM_CNTRL7_ID 296

#define NODE_ID_HND 300

#define NODE_ID_HNT0 4
#define NODE_ID_HNT1 260
#define NODE_ID_HNT2 44

#define NODE_ID_HNP0 324
#define NODE_ID_HNP1 332
#define NODE_ID_HNP2 340
#define NODE_ID_HNP3 348
#define NODE_ID_HNP4 356
#define NODE_ID_HNP5 364

#define NODE_ID_SBSX 172

#define NODE_ID_NON_PCIE_IO_MACRO NODE_ID_HNP4

#define MESH_SIZE_X 7
#define MESH_SIZE_Y 6

#define IOVB_NODE_ID0 NODE_ID_HNP0
#define IOVB_NODE_ID1 NODE_ID_HNP2
#define IOVB_NODE_ID2 NODE_ID_HNP4
#define IOVB_NODE_ID3 NODE_ID_HNT0
#define IOVB_NODE_ID4 NODE_ID_HNT2

#endif /* CMN_NODE_ID */
