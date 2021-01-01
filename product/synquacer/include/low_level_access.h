/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LOW_LEVEL_ACCESS_H
#define LOW_LEVEL_ACCESS_H

#include <stdint.h>

static inline void writeb(uint32_t ads, uint8_t data)
{
    *((volatile uint8_t *)ads) = data;
}

static inline void writew(uint32_t ads, uint16_t data)
{
    *((volatile uint16_t *)ads) = data;
}

static inline void writel(uint32_t ads, uint32_t data)
{
    *((volatile uint32_t *)ads) = data;
}

static inline void writeq(uint32_t ads, uint64_t data)
{
    *((volatile uint64_t *)ads) = data;
}

static inline uint8_t readb(uint32_t ads)
{
    return *((volatile uint8_t *)ads);
}

static inline uint16_t readw(uint32_t ads)
{
    return *((volatile uint16_t *)ads);
}

static inline uint32_t readl(uint32_t ads)
{
    return *((volatile uint32_t *)ads);
}

static inline uint64_t readq(uint32_t ads)
{
    return *((volatile uint64_t *)ads);
}

#endif /* LOW_LEVEL_ACCESS_H */
