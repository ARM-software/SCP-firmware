/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Provides <stdnoreturn.h> features missing in certain standard library
 *      implementations.
 */

#ifndef FWK_NORETURN_H
#define FWK_NORETURN_H

#ifdef __ARMCC_VERSION
#   define noreturn _Noreturn
#else
#    include <stdnoreturn.h>
#endif

#endif /* FWK_NORETURN_H */
