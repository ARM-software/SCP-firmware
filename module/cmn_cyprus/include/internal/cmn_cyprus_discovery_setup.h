/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the CMN-Cyprus discovery.
 */

#ifndef INTERNAL_CMN_CYPRUS_DISCOVERY_SETUP_H
#define INTERNAL_CMN_CYPRUS_DISCOVERY_SETUP_H

#include <internal/cmn_cyprus_ctx.h>

/*
 * Discover the topology of the interconnect and setup the context data.
 *
 * \param ctx Pointer to the driver context.
 *
 * \retval ::FWK_SUCCESS Discovery succeeded.
 * \retval ::FWK_E_DATA Discovery failed.
 */
int cmn_cyprus_discovery(struct cmn_cyprus_ctx *ctx);

#endif /* INTERNAL_CMN_CYPRUS_DISCOVERY_SETUP_H */
