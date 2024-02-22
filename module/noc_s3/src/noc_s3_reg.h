/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef NOC_S3_REG_H
#define NOC_S3_REG_H

#include <fwk_macros.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// clang-format off
/*!
 * \brief Offset address of the last register in FMU.
 */
#define FMU_ERRCIDR3_OFFSET (0xFFFC)

/*!
 * \brief NoC S3 FMU registers
 */
struct noc_s3_fmu_reg {
    FWK_R  uint64_t fmu_err_fr_0;
    FWK_RW uint64_t fmu_err_ctlr_0;
    FWK_RW uint64_t fmu_err_status;
    FWK_RW uint64_t fmu_err_addr;
    FWK_RW uint64_t fmu_err_misc0;
    FWK_RW uint64_t fmu_err_misc1;
    const  uint32_t reserved_0[4];
    FWK_R  uint64_t fmu_err_fr;
    FWK_R  uint64_t fmu_err_ctlr;
    const  uint32_t reserved_1[14316];
    FWK_R  uint64_t fmu_errgsr;
    const  uint32_t reserved_2[62];
    FWK_R  uint32_t fmu_erriidr;
    const  uint32_t reserved_3[63];
    FWK_RW uint32_t fmu_key;
    FWK_RW uint32_t fmu_smen;
    FWK_RW uint32_t fmu_sminjerr;
    const  uint32_t reserved_4;
    FWK_RW uint64_t fmu_sminfo;
    const  uint32_t reserved_5[122];
    FWK_RW uint32_t fmu_erracr;
    const  uint32_t reserved_6[1774];
    FWK_R  uint32_t fmu_errdevarch;
    const  uint32_t reserved_7[2];
    FWK_R  uint32_t fmu_errdevid;
    const  uint32_t reserved_8;
    FWK_R  uint32_t fmu_errpidr4;
    const  uint32_t reserved_9[3];
    FWK_R  uint32_t fmu_errpidr0;
    FWK_R  uint32_t fmu_errpidr1;
    FWK_R  uint32_t fmu_errpidr2;
    FWK_R  uint32_t fmu_errpidr3;
    FWK_R  uint32_t fmu_errcidr0;
    FWK_R  uint32_t fmu_errcidr1;
    FWK_R  uint32_t fmu_errcidr2;
    FWK_R  uint32_t fmu_errcidr3;
};

static_assert(
        FMU_ERRCIDR3_OFFSET == offsetof(struct noc_s3_fmu_reg, fmu_errcidr3));

/*!
 * \brief Interconnect Part Number for NoC S3.
 */

#define NOC_S3_PID0_PART_NUM (0x3F)
#define NOC_S3_PID1_PART_NUM (0x4)

/*!
 * \brief NoC S3 Domain top registers. These are common for all domains.
 */
struct noc_s3_domain_cfg_hdr {
    FWK_R uint32_t node_type;
    FWK_R uint32_t child_node_info;
    FWK_R uint32_t x_pointers[];
};

/*!  Field definitions for node_type register. */
#define NOC_S3_DOMAIN_NODE_TYPE_POS (0U)
#define NOC_S3_DOMAIN_NODE_TYPE_MSK \
    (0xFFFFUL << NOC_S3_DOMAIN_NODE_TYPE_POS)
#define NOC_S3_DOMAIN_NODE_TYPE   NOC_S3_DOMAIN_NODE_TYPE_MSK
#define NOC_S3_DOMAIN_NODE_ID_POS (16U)
#define NOC_S3_DOMAIN_NODE_ID_MSK (0xFFFFUL << NOC_S3_DOMAIN_NODE_ID_POS)
#define NOC_S3_DOMAIN_NODE_ID     NOC_S3_DOMAIN_NODE_ID_MSK

/*! Offset of first pointer register in the component configuration header. */
#define COMPONENT_CFG_HEADER_SUBFEATURE_OFFSET (0x108)

/*!
 * \brief NoC S3 Component top registers. These are common for all components.
 */
struct noc_s3_component_cfg_hdr {
    FWK_R uint32_t node_type;
    const uint32_t reserved_0[4];
    FWK_R uint32_t interface_id_0_3;
    const uint32_t reserved_1[58];
    FWK_R uint32_t num_subfeatures;
    const uint32_t reserved_2;
    struct {
        FWK_R uint32_t type;
        FWK_R uint32_t pointer;
    } subfeature[];
};

static_assert(
        COMPONENT_CFG_HEADER_SUBFEATURE_OFFSET ==
        offsetof(struct noc_s3_component_cfg_hdr, subfeature));

/*! Offset of the last register in the Global register definition. */
#define COMPONENT_ID3_OFFSET (0xFFC)

/*
 * Definitions for Global configuration peripheral id register width and shift.
 */
#define NOC_S3_GLOBAL_CFG_PERIPHERAL_ID0_PART_NUM_WIDTH 8U
#define NOC_S3_GLOBAL_CFG_PERIPHERAL_ID0_PART_NUM_SHIFT 0U
#define NOC_S3_GLOBAL_CFG_PERIPHERAL_ID0_MASK           (0x3FU)
#define NOC_S3_GLOBAL_CFG_PERIPHERAL_ID1_MASK           (0xFU)
#define NOC_S3_PART_NUMBER                              0x43F

/*!
 * \brief NoC S3 Global register.
 */
struct noc_s3_global_reg {
    FWK_R  uint32_t node_type;
    /* Number of child nodes. */
    FWK_R  uint32_t child_node_info;
    FWK_R  uint32_t vd_pointers;
    const  uint32_t reserved1[959];
    FWK_RW uint32_t secure_control;
    FWK_RW uint32_t root_control;
    const  uint32_t reserved2[48];
    FWK_R  uint32_t peripheral_id4;
    FWK_R  uint32_t peripheral_id5;
    FWK_R  uint32_t peripheral_id6;
    FWK_R  uint32_t peripheral_id7;
    FWK_R  uint32_t peripheral_id0;
    FWK_R  uint32_t peripheral_id1;
    FWK_R  uint32_t peripheral_id2;
    FWK_R  uint32_t peripheral_id3;
    FWK_R  uint32_t component_id0;
    FWK_R  uint32_t component_id1;
    FWK_R  uint32_t component_id2;
    FWK_R  uint32_t component_id3;
};

static_assert(
    COMPONENT_ID3_OFFSET ==
        offsetof(struct noc_s3_global_reg, component_id3));

// clang-format on

#endif /* NOC_S3_REG_H */
