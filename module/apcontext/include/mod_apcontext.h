
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_APCONTEXT_H
#define MOD_APCONTEXT_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupModuleAPContext AP Context
 *
 * \brief Application Processor (AP) context module.
 *
 * \details This module implements the AP context zero-initialization.
 * \{
 */

/*!
 * \brief AP context configuration data
 */
struct mod_apcontext_config {
    /*! Base address of the AP context */
    uintptr_t base;

    /*! Size of the AP context */
    size_t size;

    /*! Identifier of the clock this module depends on */
    fwk_id_t clock_id;
};

/*!
 * \}
 */

#endif /* MOD_APCONTEXT_H */
