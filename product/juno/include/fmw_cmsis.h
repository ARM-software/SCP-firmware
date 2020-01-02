/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_H
#define FMW_CMSIS_H

#define __CHECK_DEVICE_DEFINES
#define __CM3_REV 0x0201
#define __MPU_PRESENT 1
#define __NVIC_PRIO_BITS 3
#define __Vendor_SysTickConfig 0

typedef enum IRQn {
    NonMaskableInt_IRQn   = -14,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn         = -11,
    UsageFault_IRQn       = -10,
    SVCall_IRQn           = -5,
    DebugMonitor_IRQn     = -4,
    PendSV_IRQn           = -2,
    SysTick_IRQn          = -1,
} IRQn_Type;

#include <core_cm3.h>

#endif /* FMW_CMSIS_H */
