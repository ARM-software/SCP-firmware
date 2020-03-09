/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the CMN RHODES module.
 */

#ifndef CMN_RHODES_H
#define CMN_RHODES_H

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

/* Max Node Counts */
#define MAX_HNF_COUNT 64
#define MAX_RND_COUNT 32
#define MAX_RNI_COUNT 32
#define MAX_RNF_COUNT 64

/* Maximum System Cache Group regions supported by CMN-Rhodes */
#define MAX_SCG_COUNT 4

/* SAM Granularity of RN-SAM and HN-F SAM */
#define SAM_GRANULARITY       (64 * FWK_MIB)

/* External nodes that require RN-SAM mapping during run-time */
struct external_rnsam_tuple {
    unsigned int node_id;
    struct cmn_rhodes_rnsam_reg *node;
};

enum node_type {
    NODE_TYPE_INVALID   = 0x0,
    NODE_TYPE_DVM       = 0x1,
    NODE_TYPE_CFG       = 0x2,
    NODE_TYPE_DTC       = 0x3,
    NODE_TYPE_HN_I      = 0x4,
    NODE_TYPE_HN_F      = 0x5,
    NODE_TYPE_XP        = 0x6,
    NODE_TYPE_SBSX      = 0x7,
    NODE_TYPE_RN_I      = 0xA,
    NODE_TYPE_RN_D      = 0xD,
    NODE_TYPE_RN_SAM    = 0xF,
    /* Coherent Multichip Link (CML) node types */
    NODE_TYPE_CML_BASE  = 0x100,
    NODE_TYPE_CXRA      = 0x100,
    NODE_TYPE_CXHA      = 0x101,
    NODE_TYPE_CXLA      = 0x102,
};

enum device_type {
    DEVICE_TYPE_CXHA    = 0x11, // 0b10001
    DEVICE_TYPE_CXRA    = 0x12, // 0b10010
    DEVICE_TYPE_CXRH    = 0x13, // 0b10011
};

/* Common node header */
struct node_header {
    FWK_R uint64_t NODE_INFO;
          uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R uint64_t CHILD_INFO;
          uint8_t  RESERVED1[0x100 - 0x88];
    FWK_R uint64_t CHILD_POINTER[256];
};

enum sam_node_type {
    SAM_NODE_TYPE_HN_F = 0,
    SAM_NODE_TYPE_HN_I = 1,
    SAM_NODE_TYPE_CXRA = 2,
    SAM_NODE_TYPE_COUNT
};

/*
 * Request Node System Address Map (RN-SAM) registers
 */
struct cmn_rhodes_rnsam_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0xC00 - 0x908];
    FWK_RW uint64_t NON_HASH_MEM_REGION[20];
           uint8_t  RESERVED3[0xD80 - 0xCA0];
    FWK_RW uint64_t NON_HASH_TGT_NODEID[5];
           uint8_t  RESERVED4[0xE00 - 0xDA8];
    FWK_RW uint64_t SYS_CACHE_GRP_REGION[4];
           uint8_t  RESERVED5[0xEA0 - 0xE20];
    FWK_RW uint64_t SYS_CACHE_GRP_HN_COUNT;
           uint8_t  RESERVED6[0xF00 - 0xEA8];
    FWK_RW uint64_t SYS_CACHE_GRP_HN_NODEID[16];
           uint8_t  RESERVED7[0x1000 - 0xF80];
    FWK_RW uint64_t SYS_CACHE_GRP_SN_NODEID[16];
           uint8_t  RESERVED8[0x1100 - 0x1080];
    FWK_RW uint64_t STATUS;
           uint8_t  RESERVED9[0x1120 - 0x1108];
    FWK_RW uint64_t SYS_CACHE_GRP_CAL_MODE;
};

/*
 * Fully Coherent Home Node (HN-F) registers
 */
struct cmn_rhodes_hnf_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0xD00 - 0x908];
    FWK_RW uint64_t SAM_CONTROL;
    FWK_RW uint64_t SAM_MEMREGION[2];
           uint8_t  RESERVED8[0x1C00 - 0xD18];
    FWK_RW uint64_t PPU_PWPR;
};

/*
 * Configuration slave registers
 */
struct cmn_rhodes_cfgm_reg {
    FWK_R  uint64_t NODE_INFO;
    FWK_RW uint64_t PERIPH_ID[4];
    FWK_RW uint64_t COMPONENT_ID[2];
           uint8_t  RESERVED0[0x80 - 0x38];
    FWK_R  uint64_t CHILD_INFO;
};

/*
 * Crosspoint (XP) registers
 */
struct cmn_rhodes_mxp_reg {
    FWK_R  uint64_t NODE_INFO;
    FWK_R  uint64_t PORT_CONNECT_INFO[2];
           uint8_t  RESERVED0[0x80 - 0x18];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x100 - 0x88];
    FWK_R  uint64_t CHILD_POINTER[16];
};

#define CMN_RHODES_NODE_INFO_TYPE           UINT64_C(0x000000000000FFFF)
#define CMN_RHODES_NODE_INFO_ID             UINT64_C(0x00000000FFFF0000)
#define CMN_RHODES_NODE_INFO_ID_POS         16
#define CMN_RHODES_NODE_INFO_LOGICAL_ID     UINT64_C(0x0000FFFF00000000)
#define CMN_RHODES_NODE_INFO_LOGICAL_ID_POS 32

#define CMN_RHODES_CHILD_INFO_COUNT         UINT64_C(0x000000000000FFFF)

#define CMN_RHODES_CHILD_POINTER_OFFSET     UINT64_C(0x000000000FFFFFFF)
#define CMN_RHODES_CHILD_POINTER_EXT        UINT64_C(0x0000000080000000)

/* External child node */
#define CMN_RHODES_CHILD_POINTER_EXT_REGISTER_OFFSET  UINT64_C(0x00003FFF)
#define CMN_RHODES_CHILD_POINTER_EXT_NODE_POINTER     UINT64_C(0x0FFFC000)
#define CMN_RHODES_CHILD_POINTER_EXT_NODE_POINTER_POS 14

/* Used by NON_HASH_MEM_REGIONx and SYS_CACHE_GRP_REGIONx group registers */
#define CMN_RHODES_RNSAM_REGION_ENTRY_TYPE_POS 2
#define CMN_RHODES_RNSAM_REGION_ENTRY_SIZE_POS 56
#define CMN_RHODES_RNSAM_REGION_ENTRY_BASE_POS 26
#define CMN_RHODES_RNSAM_REGION_ENTRY_BITS_WIDTH 64
#define CMN_RHODES_RNSAM_REGION_ENTRY_VALID UINT64_C(0x0000000000000001)
#define CMN_RHODES_RNSAM_REGION_ENTRIES_PER_GROUP 1
#define CMN_RHODES_RNSAM_SYS_CACHE_GRP_SN_NODEID_ENTRIES_PER_GROUP 4
#define CMN_RHODES_RNSAM_SCG_HNF_CAL_MODE_EN UINT64_C(0x01)
#define CMN_RHODES_RNSAM_SCG_HNF_CAL_MODE_SHIFT 16

#define CMN_RHODES_RNSAM_STATUS_UNSTALL UINT64_C(0x0000000000000002)
#define CMN_RHODES_RNSAM_STATUS_DEFAULT_NODEID_POS 48

#define CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH 12
#define CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK UINT64_C(0xFFF)
#define CMN_RHODES_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP 4

#define CMN_RHODES_HNF_SAM_MEMREGION_SIZE_POS 12
#define CMN_RHODES_HNF_SAM_MEMREGION_BASE_POS 26
#define CMN_RHODES_HNF_SAM_MEMREGION_VALID UINT64_C(0x8000000000000000)

#define CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_MAX 64
#define CMN_RHODES_HNF_CACHE_GROUP_ENTRIES_PER_GROUP 4
#define CMN_RHODES_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH 12

#define CMN_RHODES_PPU_PWPR_POLICY_OFF UINT64_C(0x0000000000000000)
#define CMN_RHODES_PPU_PWPR_POLICY_MEM_RET UINT64_C(0x0000000000000002)
#define CMN_RHODES_PPU_PWPR_POLICY_FUNC_RET UINT64_C(0x000000000000007)
#define CMN_RHODES_PPU_PWPR_POLICY_ON UINT64_C(0x0000000000000008)
#define CMN_RHODES_PPU_PWPR_OPMODE_NOSFSLC UINT64_C(0x0000000000000000)
#define CMN_RHODES_PPU_PWPR_OPMODE_SFONLY UINT64_C(0x0000000000000010)
#define CMN_RHODES_PPU_PWPR_OPMODE_HAM UINT64_C(0x0000000000000020)
#define CMN_RHODES_PPU_PWPR_OPMODE_FAM UINT64_C(0x0000000000000030)
#define CMN_RHODES_PPU_PWPR_DYN_EN UINT64_C(0x0000000000000100)

/* Mesh and Node ID mapping */
#define CMN_RHODES_MESH_X_MAX 8
#define CMN_RHODES_MESH_Y_MAX 8

#define CMN_RHODES_NODE_ID_PORT_POS 2
#define CMN_RHODES_NODE_ID_PORT_MASK 0x1
#define CMN_RHODES_NODE_ID_Y_POS 3

#define CMN_RHODES_MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK UINT64_C(0x1F)

#define CMN_RHODES_ROOT_NODE_OFFSET_PORT_POS 16
#define CMN_RHODES_ROOT_NODE_OFFSET_Y_POS 22

/*
 * Retrieve the number of child nodes of a given node
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 *
 * \return Number of child nodes
 */
unsigned int get_node_child_count(void *node_base);

/*
 * Retrieve node type identifier
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 *
 * \return Node's type identifier
 */
enum node_type get_node_type(void *node_base);

/*
 * Retrieve the physical identifier of a node from its hardware node descriptor.
 * This identifier encodes the node's position in the mesh.
 *
 * Note: Multiple node descriptors can share the same identifier if they are
 * related to the same device node in the mesh.
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 *
 * \return Node's physical identifier
 */
unsigned int get_node_id(void *node_base);

/*
 * Retrieve the logical identifier of a node from its hardware node descriptor.
 * This is an unique identifier (index) among nodes of the same type in the
 * system.
 *
 * \param node_base Pointer to the node base address
 *      \pre The node pointer must be valid
 *
 * \return An integer representing the node's logical identifier
 */
unsigned int get_node_logical_id(void *node_base);

/*
 * Retrieve a child node given a node and child index
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 * \param child_index Child index
 *      \pre The child index must be valid
 *
 * \return Pointer to the child's node descriptor
 */
void *get_child_node(uintptr_t base, void *node_base, unsigned int child_index);

/*
 * Retrieve the physical identifier of a node using its child pointer in the
 * parent's node hardware descriptor
 *
 * This function is used to extract a node's identifier without accessing the
 * node descriptor. This is specially useful for external nodes that are in an
 * unavailable power or clock domain.
 *
 * \param node_base Pointer to the parent node descriptor
 *      \pre The node pointer must be valid
 * \param child_index Child index
 *      \pre The child index must be valid
 *
 * \return Physical child node identifier
 */
unsigned int get_child_node_id(void *node_base, unsigned int child_index);

/*
 * Verify if a child node (given a parent node base and child index) is an
 * external node from the CMN RHODES instance point of view.
 *
 * \param node_base Pointer to the parent node descriptor
 *      \pre The node pointer must be valid
 * \param child_index Child index
 *      \pre The child index must be valid
 *
 * \retval true if the node is external
 * \retval false if the node is internal
 */
bool is_child_external(void *node_base, unsigned int child_index);

/*
 * Returns the port number from the child node id.
 *
 * \param child_node_id Child node id calculated from the child pointer.
 *
 * \retval port number (either 0 or 1).
 */
bool get_port_number(unsigned int child_node_id);

/*
 * Returns the device type from the MXP's port connect info register.
 *
 * \param mxp_base Pointer to the cross point node descriptor
 *      \pre The cross point node pointer must be valid
 * \param port Port number
 *      \pre The port number should be either 0 or 1.
 *
 * \retval device type (por_mxp_por_mxp_device_port_connect_info_p[port] & 0x1F)
 */
unsigned int get_device_type(void *mxp_base, bool port);

/*
 * Convert a memory region size into a size format used by the CMN RHODES
 * registers. The format is the binary logarithm of the memory region size
 * represented as blocks multiple of the CMN RHODES's granularity:
 * n =  log2(size / SAM_GRANULARITY)
 *
 * \param size Memory region size to be converted
 *      \pre size must be a multiple of SAM_GRANULARITY
 *
 * \return log2(size / SAM_GRANULARITY)
 */
uint64_t sam_encode_region_size(uint64_t size);

/*
 * Configure a memory region
 *
 * \param reg Pointer to the region group descriptor to be configured
 *      \pre Must be a valid pointer
 * \param region Region entry in the region group descriptor
 * \param base Region base address
 * \param size Region size
 * \param node_type Type of the target node
 *
 * \return None
 */
void configure_region(volatile uint64_t *reg, uint64_t base, uint64_t size,
    enum sam_node_type node_type);

/*
 * Retrieve the node type name
 *
 * \param node_type Node type
 *
 * \return Pointer to the node type name string
 */
const char *get_node_type_name(enum node_type node_type);

/*
 * Retrieve the node's position in the mesh along the X-axis
 *
 * \param node_base Pointer to the node descriptor
 *
 * \return Zero-indexed position along the X-axis
 */
unsigned int get_node_pos_x(void *node_base);

/*
 * Retrieve the node's position in the mesh along the Y-axis
 *
 * \param node_base Pointer to the node descriptor
 *
 * \return Zero-indexed position along the Y-axis
 */
unsigned int get_node_pos_y(void *node_base);

/*
 * Get the root node descriptor based on the peripheral base, HN-D node
 * identifier and mesh size.
 *
 * \param base CMN RHODES peripheral base address
 * \param hnd_node_id HN-D node identifier containing the global configuration
 * \param mesh_size_x Size of the mesh along the x-axis
 * \param mesh_size_y Size of the mesh along the x-axis
 *
 * \return Pointer to the root node descriptor
 */
struct cmn_rhodes_cfgm_reg *get_root_node(uintptr_t base,
    unsigned int hnd_node_id, unsigned int mesh_size_x,
    unsigned int mesh_size_y);

#endif /* CMN_RHODES_H */
