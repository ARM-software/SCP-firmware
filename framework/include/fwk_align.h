/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
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
#   include <stdalign.h>
#endif

#ifdef BUILD_OPTEE
#include <stddef.h>
#ifndef _GCC_MAX_ALIGN_T
typedef uintmax_t max_align_t;
#endif
#endif /*BUILD_OPTEE*/

#endif /* FWK_ALIGN_H */
