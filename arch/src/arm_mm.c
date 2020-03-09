/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory initialization.
 */

#include <fwk_arch.h>
#include <fwk_assert.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

int arm_mm_init(struct fwk_arch_mm_data *data)
{
    fwk_assert(data != NULL);

#ifdef __ARMCC_VERSION
    extern unsigned int Image$$ARM_LIB_STACKHEAP$$ZI$$Base;
    extern unsigned int Image$$ARM_LIB_STACKHEAP$$ZI$$Length;

    data->start = (uintptr_t)(&Image$$ARM_LIB_STACKHEAP$$ZI$$Base);
    data->size = (size_t)(&Image$$ARM_LIB_STACKHEAP$$ZI$$Length);
#else
    extern char __stackheap_start__;
    extern char __stackheap_end__;

    uintptr_t start = (uintptr_t)(&__stackheap_start__);
    uintptr_t end = (uintptr_t)(&__stackheap_end__);

    data->start = start;
    data->size = end - start;
#endif

    return FWK_SUCCESS;
}
