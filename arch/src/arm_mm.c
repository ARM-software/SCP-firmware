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
#include <section.h>

int arm_mm_init(struct fwk_arch_mm_data *data)
{
    const unsigned int ALIGNMENT = sizeof(uint64_t);

    uintptr_t start;
    uintptr_t end;

    if (data == NULL)
        return FWK_E_PARAM;

    if (ARCH_SECTION_HEAP_SIZE == 0)
        return FWK_E_NOMEM;

    /* Check if the main stack section overlaps the .bss section */
    if (ARCH_SECTION_STACK_START <= ARCH_SECTION_BSS_END) {
        assert(false);
        return FWK_E_PANIC;
    }

    /* Ensure the start address did not overflow following 64-bit alignment */
    start = FWK_ALIGN_NEXT(ARCH_SECTION_HEAP_START, ALIGNMENT);
    if (start < ARCH_SECTION_HEAP_START)
        return FWK_E_NOMEM;

    /* Ensure the end address did not underflow following 64-bit alignment */
    end = FWK_ALIGN_PREVIOUS(ARCH_SECTION_HEAP_END, ALIGNMENT);
    if (end > ARCH_SECTION_HEAP_END)
        return FWK_E_NOMEM;

    /*
     * Ensure there is space left after performing 64-bit alignment on the start
     * and end addresses.
     */
    if (end <= start)
        return FWK_E_NOMEM;

    data->start = start;
    data->size = end - start;

    return FWK_SUCCESS;
}
