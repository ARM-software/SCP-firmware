/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STDLIB_H
#define STDLIB_H

#include <fwk_attributes.h>

#include_next <stdlib.h>

#ifdef __ARMCC_VERSION
void *aligned_alloc(size_t alignment, size_t size) FWK_LEAF
    FWK_NOTHROW FWK_ALLOC FWK_ALLOC_ALIGN(1) FWK_ALLOC_SIZE1(2) FWK_WARN_UNUSED;
#endif

#endif /* STDLIB_H */
