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

extern unsigned int __text_start__;
extern unsigned int __text_size__;
extern unsigned int __text_end__;

extern unsigned int __data_start__;
extern unsigned int __data_size__;
extern unsigned int __data_end__;

extern unsigned int __bss_start__;
extern unsigned int __bss_size__;
extern unsigned int __bss_end__;

extern unsigned int __heap_start__;
extern unsigned int __heap_size__;
extern unsigned int __heap_end__;

extern unsigned int __stack_start__;
extern unsigned int __stack_size__;
extern unsigned int __stack_end__;

/* Beginning of the .text section */
#define ARCH_SECTION_TEXT_START ((uintptr_t)(&__text_start__))

/* End of the .text section */
#define ARCH_SECTION_TEXT_END ((uintptr_t)(&__text_end__))

/* Size of the .text section */
#define ARCH_SECTION_TEXT_SIZE ((size_t)(&__text_size__))

/* Start of the .data section */
#define ARCH_SECTION_DATA_START ((uintptr_t)(&__data_start__))

/* End of the .data section */
#define ARCH_SECTION_DATA_END ((uintptr_t)(&__data_end__))

/* Size of the .data section */
#define ARCH_SECTION_DATA_SIZE ((size_t)(&__data_size__))

/* Start of the .bss section */
#define ARCH_SECTION_BSS_START ((uintptr_t)(&__bss_start__))

/* End of the .bss section */
#define ARCH_SECTION_BSS_END ((uintptr_t)(&__bss_end__))

/* Size of the .bss section */
#define ARCH_SECTION_BSS_SIZE ((size_t)(&__bss_size__))

/* Start of the .heap section */
#define ARCH_SECTION_HEAP_START ((uintptr_t)(&__heap_start__))

/* End of the .heap section */
#define ARCH_SECTION_HEAP_END ((uintptr_t)(&__heap_end__))

/* Size of the .heap section */
#define ARCH_SECTION_HEAP_SIZE ((size_t)(&__heap_size__))

/* Start of the .stack section */
#define ARCH_SECTION_STACK_START ((uintptr_t)(&__stack_start__))

/* End of the .stack section */
#define ARCH_SECTION_STACK_END ((uintptr_t)(&__stack_end__))

/* Size of the .stack section */
#define ARCH_SECTION_STACK_SIZE ((size_t)(&__stack_size__))

#endif /* ARCH_SECTION_H */
