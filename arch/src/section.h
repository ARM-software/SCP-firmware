/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Section symbols.
 */

#ifndef ARCH_SECTION_H
#define ARCH_SECTION_H

#include <stddef.h>

extern unsigned int __TEXT_START__;
extern unsigned int __TEXT_SIZE__;
extern unsigned int __TEXT_END__;

extern unsigned int __DATA_START__;
extern unsigned int __DATA_SIZE__;
extern unsigned int __DATA_END__;

extern unsigned int __BSS_START__;
extern unsigned int __BSS_SIZE__;
extern unsigned int __BSS_END__;

extern unsigned int __STACK_START__;
extern unsigned int __STACK_SIZE__;
extern unsigned int __STACK_END__;

/* Beginning of the .text section */
#define ARCH_SECTION_TEXT_START ((uintptr_t)(&__TEXT_START__))

/* End of the .text section */
#define ARCH_SECTION_TEXT_END ((uintptr_t)(&__TEXT_END__))

/* Size of the .text section */
#define ARCH_SECTION_TEXT_SIZE ((size_t)(&__TEXT_SIZE__))

/* Start of the .data section */
#define ARCH_SECTION_DATA_START ((uintptr_t)(&__DATA_START__))

/* End of the .data section */
#define ARCH_SECTION_DATA_END ((uintptr_t)(&__DATA_END__))

/* Size of the .data section */
#define ARCH_SECTION_DATA_SIZE ((size_t)(&__DATA_SIZE__))

/* Start of the .bss section */
#define ARCH_SECTION_BSS_START ((uintptr_t)(&__BSS_START__))

/* End of the .bss section */
#define ARCH_SECTION_BSS_END ((uintptr_t)(&__BSS_END__))

/* Size of the .bss section */
#define ARCH_SECTION_BSS_SIZE ((size_t)(&__BSS_SIZE__))

/* Start of the .heap section */
#define ARCH_SECTION_HEAP_START ARCH_SECTION_BSS_END

/* End of the .heap section */
#define ARCH_SECTION_HEAP_END ARCH_SECTION_STACK_START

/* Size of the .heap section */
#define ARCH_SECTION_HEAP_SIZE ((size_t)(ARCH_SECTION_HEAP_END - \
                                         ARCH_SECTION_HEAP_START))

/* Start of the .stack section */
#define ARCH_SECTION_STACK_START ((uintptr_t)(&__STACK_START__))

/* End of the .stack section */
#define ARCH_SECTION_STACK_END ((uintptr_t)(&__STACK_END__))

/* Size of the .stack section */
#define ARCH_SECTION_STACK_SIZE ((size_t)(&__STACK_SIZE__))

#endif /* ARCH_SECTION_H */
