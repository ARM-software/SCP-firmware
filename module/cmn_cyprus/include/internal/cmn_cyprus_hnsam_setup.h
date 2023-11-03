/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for the programming HN-SAM.
 */

#ifndef CMN_CYPRUS_HNSAM_SETUP_INTERNAL_H
#define CMN_CYPRUS_HNSAM_SETUP_INTERNAL_H

#include <internal/cmn_cyprus_ctx.h>

/*
 * Program the HN-F SAM.
 *
 * \param ctx Pointer to the driver context.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_DATA Invalid configuration data.
 */
int cmn_cyprus_setup_hnf_sam(struct cmn_cyprus_ctx *ctx);

#endif /* CMN_CYPRUS_HNSAM_SETUP_INTERNAL_H */
