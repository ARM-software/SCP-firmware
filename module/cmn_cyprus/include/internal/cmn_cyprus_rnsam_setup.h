/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the programming CMN-Cyprus RNSAM.
 */

#ifndef CMN_CYPRUS_RNSAM_SETUP_INTERNAL_H
#define CMN_CYPRUS_RNSAM_SETUP_INTERNAL_H

#include <internal/cmn_cyprus_ctx.h>

/*
 * Program the RNSAM.
 *
 * \param ctx Pointer to the driver context.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_DATA Invalid configuration data.
 */
int cmn_cyprus_setup_rnsam(struct cmn_cyprus_ctx *ctx);

/*
 * Get RNSAM mmap API.
 *
 * \param[out] api Pointer to the API pointer variable.
 *
 * \return None.
 */
void get_rnsam_memmap_api(const void **api);

#endif /* CMN_CYPRUS_RNSAM_SETUP_INTERNAL_H */
