/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_MHU_H
#define INTERNAL_MHU_H

#include <mod_mhu.h>

#include <fwk_macros.h>

#include <stdint.h>

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

#endif /* INTERNAL_MHU_H */
