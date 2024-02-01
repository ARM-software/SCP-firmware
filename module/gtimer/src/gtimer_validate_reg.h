/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GTIMER_VALIDATE_REG_H
#define GTIMER_VALIDATE_REG_H

#include "gtimer_reg.h"

static_assert(CNTCR_OFFSET == offsetof(struct cntcontrol_reg, CR));
static_assert(CNTSR_OFFSET == offsetof(struct cntcontrol_reg, SR));
static_assert(CNTCV_L_OFFSET == offsetof(struct cntcontrol_reg, CVL));
static_assert(CNTCV_H_OFFSET == offsetof(struct cntcontrol_reg, CVH));
static_assert(CNTSCR_OFFSET == offsetof(struct cntcontrol_reg, CSR));
static_assert(CNTID_OFFSET == offsetof(struct cntcontrol_reg, ID));
static_assert(CNTFID0_OFFSET == offsetof(struct cntcontrol_reg, FID0));
static_assert(IMP_DEF_OFFSET == offsetof(struct cntcontrol_reg, IMP_DEF));
static_assert(COUNTERID_OFFSET == offsetof(struct cntcontrol_reg, PID));

#endif /* GTIMER_VALIDATE_REG_H */
