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

extern unsigned int __stackheap_start__;
extern unsigned int __stackheap_size__;

int arm_mm_init(struct fwk_arch_mm_data *data)
{
    fwk_assert(data != NULL);

    data->start = (uintptr_t)(&__stackheap_start__);
    data->size = (size_t)(&__stackheap_size__);

    return FWK_SUCCESS;
}
