/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_WDOG_ROM_H
#define JUNO_WDOG_ROM_H

#include <stdint.h>

void juno_wdog_rom_enable(void);
void juno_wdog_rom_reload(void);

#endif /* JUNO_WDOG_ROM_H */
