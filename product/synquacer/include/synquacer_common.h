/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_COMMON_H
#define SYNQUACER_COMMON_H

#ifdef HAS_RTOS
#    include <cmsis_os2.h>
#endif

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>

#define DI(intsts)                  \
    do {                            \
        intsts = __get_FAULTMASK(); \
        __disable_fault_irq();      \
    } while (0)

#define EI(intsts)                 \
    do {                           \
        if ((intsts & 0x1) == 0) { \
            __enable_fault_irq();  \
        }                          \
    } while (0)

#endif /* SYNQUACER_COMMON_H */
