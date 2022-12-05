/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     String management.
 */

#include <fwk_assert.h>
#include <fwk_mm.h>
#include <fwk_string.h>

#include <stdlib.h>

void fwk_str_memset(void *dest, int ch, size_t count)
{
    void *ret;

    if (dest == NULL) {
        fwk_trap();
    }

    ret = memset(dest, ch, count);
    if (ret != dest) {
        fwk_trap();
    }
}

void fwk_str_memcpy(void *dest, const void *src, size_t count)
{
    void *ret;

    if ((dest == NULL) || (src == NULL)) {
        fwk_trap();
    }

    ret = memcpy(dest, src, count);
    if (ret != dest) {
        fwk_trap();
    }
}

void fwk_str_strncpy(char *dest, const char *src, size_t count)
{
    char *ch;

    if ((dest == NULL) || (src == NULL)) {
        fwk_trap();
    }

    ch = strncpy(dest, src, count);
    if (ch != dest) {
        fwk_trap();
    }
}
