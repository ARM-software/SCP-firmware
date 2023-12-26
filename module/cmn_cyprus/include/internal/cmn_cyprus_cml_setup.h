/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for programming the Coherent Multichip Link (CML).
 */

#ifndef CMN_CYPRUS_CML_SETUP_INTERNAL_H
#define CMN_CYPRUS_CML_SETUP_INTERNAL_H

#include <internal/cmn_cyprus_ctx.h>

/*
 * Setup coherent multichip links.
 *
 * \param ctx Pointer to the driver context.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int cmn_cyprus_setup_cml(struct cmn_cyprus_ctx *ctx);

#endif /* CMN_CYPRUS_CML_SETUP_INTERNAL_H */
