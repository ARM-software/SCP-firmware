/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_PWC_H
#define RCAR_PWC_H

void rcar_pwrc_go_suspend_to_ram(void);
void rcar_pwrc_set_suspend_to_ram(void);
void rcar_system_off(void);
void rcar_system_reset(void);

#endif /* RCAR_PWC_H */
