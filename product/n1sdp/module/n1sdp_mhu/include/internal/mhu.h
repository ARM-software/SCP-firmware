/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MHU_H
#define MHU_H

#include <stdint.h>
#include <fwk_macros.h>
#include <mod_mhu.h>

/*!
 * \brief MHU Register Definitions
 */
struct mhu_reg {
    /*! Status register */
    FWK_R  uint32_t STAT;
           uint32_t RESERVED0;
    /*! Set register (sets the value of STAT) */
    FWK_W  uint32_t SET;
           uint32_t RESERVED1;
    /*! Clear register (clears STAT) */
    FWK_W  uint32_t CLEAR;
};

#endif /* MHU_H */
