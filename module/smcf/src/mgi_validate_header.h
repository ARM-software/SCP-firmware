/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MGI_VALIDATE_HEADER_H
#define MGI_VALIDATE_HEADER_H

#include "mgi.h"

static_assert(MGI_GRP_ID_OFFSET == offsetof(struct smcf_mgi_reg, GRP_ID));
static_assert(MGI_DATA_INFO_OFFSET == offsetof(struct smcf_mgi_reg, DATA_INFO));
static_assert(MGI_FEAT0_OFFSET == offsetof(struct smcf_mgi_reg, FEAT0));
static_assert(MGI_FEAT1_OFFSET == offsetof(struct smcf_mgi_reg, FEAT1));
static_assert(MGI_SMP_EN_OFFSET == offsetof(struct smcf_mgi_reg, SMP_EN));
static_assert(MGI_SMP_CFG_OFFSET == offsetof(struct smcf_mgi_reg, SMP_CFG));
static_assert(MGI_SMP_PER_OFFSET == offsetof(struct smcf_mgi_reg, SMP_PER));
static_assert(MGI_SMP_DLY_OFFSET == offsetof(struct smcf_mgi_reg, SMP_DLY));
static_assert(MGI_MON_REQ_OFFSET == offsetof(struct smcf_mgi_reg, MON_REQ));
static_assert(MGI_MON_STAT_OFFSET == offsetof(struct smcf_mgi_reg, MON_STAT));
static_assert(
    MGI_MODE_BCAST_OFFSET == offsetof(struct smcf_mgi_reg, MODE_BCAST));
static_assert(MGI_MODE_REQ0_OFFSET == offsetof(struct smcf_mgi_reg, MODE_REQ0));
static_assert(MGI_MODE_REQ1_OFFSET == offsetof(struct smcf_mgi_reg, MODE_REQ1));
static_assert(MGI_MODE_REQ2_OFFSET == offsetof(struct smcf_mgi_reg, MODE_REQ2));
static_assert(MGI_MODE_REQ3_OFFSET == offsetof(struct smcf_mgi_reg, MODE_REQ3));
static_assert(
    MGI_MODE_STAT0_OFFSET == offsetof(struct smcf_mgi_reg, MODE_STAT0));
static_assert(
    MGI_MODE_STAT1_OFFSET == offsetof(struct smcf_mgi_reg, MODE_STAT1));
static_assert(
    MGI_MODE_STAT2_OFFSET == offsetof(struct smcf_mgi_reg, MODE_STAT2));
static_assert(
    MGI_MODE_STAT3_OFFSET == offsetof(struct smcf_mgi_reg, MODE_STAT3));
static_assert(MGI_IRQ_STAT_OFFSET == offsetof(struct smcf_mgi_reg, IRQ_STAT));
static_assert(MGI_IRQ_MASK_OFFSET == offsetof(struct smcf_mgi_reg, IRQ_MASK));
static_assert(MGI_TRG_MASK_OFFSET == offsetof(struct smcf_mgi_reg, TRG_MASK));
static_assert(MGI_ERR_CODE_OFFSET == offsetof(struct smcf_mgi_reg, ERR_CODE));
static_assert(MGI_WREN_OFFSET == offsetof(struct smcf_mgi_reg, WREN));
static_assert(MGI_WRCFG_OFFSET == offsetof(struct smcf_mgi_reg, WRCFG));
static_assert(MGI_WADDR0_OFFSET == offsetof(struct smcf_mgi_reg, WADDR0));
static_assert(MGI_WADDR1_OFFSET == offsetof(struct smcf_mgi_reg, WADDR1));
static_assert(MGI_RADDR0_OFFSET == offsetof(struct smcf_mgi_reg, RADDR0));
static_assert(MGI_RADDR1_OFFSET == offsetof(struct smcf_mgi_reg, RADDR1));
static_assert(MGI_DISCON_ID_OFFSET == offsetof(struct smcf_mgi_reg, DISCON_ID));
static_assert(MGI_CON_STAT_OFFSET == offsetof(struct smcf_mgi_reg, CON_STAT));
static_assert(MGI_CMD_SEND0_OFFSET == offsetof(struct smcf_mgi_reg, CMD_SEND0));
static_assert(MGI_CMD_SEND1_OFFSET == offsetof(struct smcf_mgi_reg, CMD_SEND1));
static_assert(MGI_CMD_RECV0_OFFSET == offsetof(struct smcf_mgi_reg, CMD_RECV0));
static_assert(MGI_CMD_RECV1_OFFSET == offsetof(struct smcf_mgi_reg, CMD_RECV1));
static_assert(MGI_ATYP0_OFFSET == offsetof(struct smcf_mgi_reg, ATYP0));
static_assert(MGI_ATYP1_OFFSET == offsetof(struct smcf_mgi_reg, ATYP1));
static_assert(MGI_AVAL_LOW0_OFFSET == offsetof(struct smcf_mgi_reg, AVAL_LOW0));
static_assert(MGI_AVAL_LOW1_OFFSET == offsetof(struct smcf_mgi_reg, AVAL_LOW1));
static_assert(MGI_AVAL_LOW2_OFFSET == offsetof(struct smcf_mgi_reg, AVAL_LOW2));
static_assert(MGI_AVAL_LOW3_OFFSET == offsetof(struct smcf_mgi_reg, AVAL_LOW3));
static_assert(
    MGI_AVAL_HIGH6_OFFSET == offsetof(struct smcf_mgi_reg, AVAL_HIGH6));
static_assert(MGI_DATA_OFFSET == offsetof(struct smcf_mgi_reg, DATA));
static_assert(MGI_DVLD_OFFSET == offsetof(struct smcf_mgi_reg, DVLD));
static_assert(MGI_TAG0_OFFSET == offsetof(struct smcf_mgi_reg, TAG0));
static_assert(MGI_TAG1_OFFSET == offsetof(struct smcf_mgi_reg, TAG1));
static_assert(
    MGI_SMPID_START_OFFSET == offsetof(struct smcf_mgi_reg, SMPID_START));
static_assert(MGI_SMPID_END_OFFSET == offsetof(struct smcf_mgi_reg, SMPID_END));
static_assert(MGI_IIDR_OFFSET == offsetof(struct smcf_mgi_reg, IIDR));
static_assert(MGI_AIDR_OFFSET == offsetof(struct smcf_mgi_reg, AIDR));

#endif /* MGI_VALIDATE_HEADER_H */
