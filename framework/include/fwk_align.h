/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Provides <stdalign.h> features missing in certain standard library
 *      implementations.
 */

#ifndef FWK_ALIGN_H
#define FWK_ALIGN_H

#ifdef __ARMCC_VERSION
#   define alignas _Alignas
#   define alignof _Alignof

#   define __alignas_is_defined 1
#   define __alignof_is_defined 1
#else
#    include <stdalign.h>
#endif

#endif /* FWK_ALIGN_H */
