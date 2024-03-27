/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APREMAP_H
#define APREMAP_H

#define APREMAP_CMN_ATRANS_EN_MASK 0x80000000
#define APREMAP_CMN_ATRANS_EN_POS  31

#define APREMAP_ADDR_TRANS_EN            UINT32_C(1)
#define APREMAP_ADDR_TRANS_AP_ADDR_SHIFT 20

#define APREMAP_1MB_ADDR(addr) (0xCB000000 + ((uintptr_t)addr & 0xFFFFF))

#define SYSTEM_ACCESS_PORT_0_BASE 0x60000000
#define SYSTEM_ACCESS_PORT_1_BASE 0xA0000000

#define ADDR_OFFSET_SYSTEM_ACCESS_PORT_0(addr) \
    ((uintptr_t)addr + (SYSTEM_ACCESS_PORT_0_BASE - (1 * FWK_GIB)))
#define ADDR_OFFSET_SYSTEM_ACCESS_PORT_1(addr) \
    ((uintptr_t)addr + SYSTEM_ACCESS_PORT_1_BASE)

/* Data Type enum used internally within the module */
typedef enum mod_apremap_type_idx {
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
} mod_apremap_type_idx_t;

#endif /* APREMAP_H */
