/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STRING_H
#define STRING_H

typedef unsigned long size_t;

void *memset(void *s, int c, size_t count);
void *memcpy(void *dst, const void *src, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *str, int c);
size_t strlen(const char *str);

#endif /* STRING_H */
