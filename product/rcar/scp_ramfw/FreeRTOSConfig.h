/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_IDLE_HOOK 1
#define configUSE_TICK_HOOK 0
#define configMINIMAL_STACK_SIZE ((unsigned short)200)
#define configTOTAL_HEAP_SIZE ((size_t)(20 * 1024))

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 5
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2)

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskDelay 1

#define INCLUDE_xSemaphoreGetMutexHolder 1

void vConfigureTickInterrupt(void);
#define configSETUP_TICK_INTERRUPT() vConfigureTickInterrupt()
void vClearTickInterrupt(void);
#define configCLEAR_TICK_INTERRUPT() vClearTickInterrupt()

/* Defines needed by FreeRTOS to implement CMSIS RTOS2 API. Do not change! */
#define configTICK_RATE_HZ ((TickType_t)1000 /* 1ms */)
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configUSE_PREEMPTION 0
#define configUSE_TIMERS 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_TRACE_FACILITY 1
#define configUSE_16_BIT_TICKS 0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES 56
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_OS2_THREAD_FLAGS 1
#endif /* FREERTOS_CONFIG_H */
