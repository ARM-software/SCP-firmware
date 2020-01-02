/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <synquacer_debug.h>
#include <cmsis_compiler.h>
#include <rtx_lib.c>
#include <rtx_os.h>
#include <system_clock.h>

/*
 * Required by RTX to configure the SysTick timer.
 */
uint32_t SystemCoreClock = CLOCK_RATE_REFCLK;

/*
 * Idle thread
 */
__NO_RETURN void osRtxIdleThread(void *argument)
{
    (void)argument;

    while (true)
        __WFI();
}

/*
 * OS error handler
 */
uint32_t osRtxErrorNotify(uint32_t code, void *object_id)
{
    (void)object_id;

    switch (code) {
    case osRtxErrorStackUnderflow:
        /*
         * Stack underflow detected for thread
         * thread_id=object_id
         */
        SYNQUACER_DEV_LOG_ERROR("[SYSTEM] osRtxErrorStackUnderflow.\n");
        break;

    case osRtxErrorISRQueueOverflow:
        /*
         * ISR Queue overflow detected when inserting object
         * object_id
         */
        SYNQUACER_DEV_LOG_ERROR("[SYSTEM] osRtxErrorISRQueueOverflow.\n");
        break;

    case osRtxErrorTimerQueueOverflow:
        /*
         * User Timer Callback Queue overflow detected for timer
         * timer_id=object_id
         */
        SYNQUACER_DEV_LOG_ERROR("[SYSTEM] osRtxErrorTimerQueueOverflow.\n");
        break;

    case osRtxErrorClibSpace:
        /*
         * Standard C/C++ library libspace not available:
         * increase OS_THREAD_LIBSPACE_NUM
         */
        SYNQUACER_DEV_LOG_ERROR("[SYSTEM] osRtxErrorClibSpace.\n");
        break;

    case osRtxErrorClibMutex:
        /*
         * Standard C/C++ library mutex initialization failed
         */
        SYNQUACER_DEV_LOG_ERROR("[SYSTEM] oosRtxErrorClibMutex.\n");
        break;

    default:
        break;
    }

    osRtxIdleThread(object_id);
}
