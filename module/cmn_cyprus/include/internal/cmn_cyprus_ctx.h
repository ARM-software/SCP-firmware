/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN Cyprus driver module context structure.
 */

#ifndef INTERNAL_CMN_CYPRUS_CTX_H
#define INTERNAL_CMN_CYPRUS_CTX_H

#include <mod_cmn_cyprus.h>

/*!
 * \brief CMN Cyprus driver context.
 */
struct cmn_cyprus_ctx {
    /*! CMN Cyprus driver configuration data */
    const struct mod_cmn_cyprus_config *config;
};

#endif /* INTERNAL_CMN_CYPRUS_CTX_H */
