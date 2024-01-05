/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for the programming LCN-SAM.
 */

#ifndef CMN_CYPRUS_LCNSAM_SETUP_INTERNAL_H
#define CMN_CYPRUS_LCNSAM_SETUP_INTERNAL_H

#include <internal/cmn_cyprus_ctx.h>

/*
 * Program the LCN SAM.
 *
 * \param ctx Pointer to the driver context.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int cmn_cyprus_setup_lcn_sam(struct cmn_cyprus_ctx *ctx);

#endif /* CMN_CYPRUS_LCNSAM_SETUP_INTERNAL_H */
