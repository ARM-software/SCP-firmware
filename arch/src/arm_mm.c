/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory initialization.
 */

#include <stdint.h>
#include <fwk_arch.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_macros.h>

int arm_mm_init(struct fwk_arch_mm_data *data)
{
    fwk_assert(data != NULL);

#ifdef __ARMCC_VERSION
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"

    extern unsigned int Image$$ARM_LIB_STACKHEAP$$ZI$$Base;
    extern unsigned int Image$$ARM_LIB_STACKHEAP$$ZI$$Length;

    data->start = (uintptr_t)(&Image$$ARM_LIB_STACKHEAP$$ZI$$Base);
    data->size = (size_t)(&Image$$ARM_LIB_STACKHEAP$$ZI$$Length);

#   pragma clang diagnostic pop
#else
    extern unsigned int __stackheap_start__;
    extern unsigned int __stackheap_size__;

    data->start = (uintptr_t)(&__stackheap_start__);
    data->size = (size_t)(&__stackheap_size__);
#endif

    return FWK_SUCCESS;
}
