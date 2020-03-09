/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_DEBUG_ROM_H
#define JUNO_DEBUG_ROM_H

#include <mod_juno_ppu.h>

#include <stdint.h>

int juno_debug_rom_init(const struct mod_juno_ppu_rom_api *rom_ppu_api);

#endif /* JUNO_DEBUG_ROM_H */
