/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      C runtime initialization for Arm Compiler 6.
 */

    .syntax unified

    .text

    .global exception_reset
    .thumb_func
    .type exception_reset, %function
exception_reset:
    bl  arm_init_ccr /* Initialize the CCR register */
    b   __main /* Enter Arm Compiler 6's crt0 */
    .pool
