/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_ARCH_H
#define FMW_ARCH_H
/*
 * Suspend feature is disabled until the whole system
 * ( LCP,MCP and SCPs ) become available to wakeup
 * the LCP.
 */
#define FMW_DISABLE_ARCH_SUSPEND 1

#endif /* FMW_ARCH_H */
