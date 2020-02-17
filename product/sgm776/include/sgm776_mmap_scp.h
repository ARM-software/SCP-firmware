/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP ROM and RAM memory bases. These definitions are kept isolated without
 *     the UINT32_C() or UL decorators allowing them to be used in the linker
 *     script.
 */

#ifndef SGM776_MMAP_SCP_H
#define SGM776_MMAP_SCP_H

#define SCP_ROM_BASE  0x00000000
#define SCP_RAM_BASE  0x10000000

#endif /* SGM776_MMAP_SCP_H */
