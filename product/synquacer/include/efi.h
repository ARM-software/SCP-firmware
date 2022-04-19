/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_EFI_H
#define SYNQUACER_EFI_H

typedef struct {
        uint8_t b[16];
} efi_guid_t __attribute__((aligned(8)));

#endif /* SYNQUACER_EFI_H */
