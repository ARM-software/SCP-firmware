/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2019 Arm Limited. All rights reserved.
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 *reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    cmsis_os2_tyny4scp.c
 *      Purpose: CMSIS RTOS2 wrapper for FreeRTOS
 *
 *---------------------------------------------------------------------------*/

#include "FreeRTOS.h" // ARM.FreeRTOS::RTOS:Core
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "task.h" // ARM.FreeRTOS::RTOS:Core

#include <string.h>

#define __ARM_ARCH_8A__ (1)

#define IS_IRQ() is_irq()

/* Kernel version and identification string definition
                                    (major.minor.rev: mmnnnrrrr dec) */
#define KERNEL_VERSION \
    (((uint32_t)tskKERNEL_VERSION_MAJOR * 10000000UL) | \
     ((uint32_t)tskKERNEL_VERSION_MINOR * 10000UL) | \
     ((uint32_t)tskKERNEL_VERSION_BUILD * 1UL))

#define KERNEL_ID ("FreeRTOS " tskKERNEL_VERSION_NUMBER)

#define portYIELD_FROM_ISR(n)
#define __STATIC_INLINE static inline
/* Limits */
#define MAX_BITS_TASK_NOTIFY 31U
#define MAX_BITS_EVENT_GROUPS 24U

#define THREAD_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_TASK_NOTIFY) - 1U))
#define EVENT_FLAGS_INVALID_BITS (~((1UL << MAX_BITS_EVENT_GROUPS) - 1U))

static inline uint32_t is_irq(void)
{
    uint32_t val;
    __asm__ volatile("mrs %0, spsel" : "=r"(val));
    return val & 0x01;
}

/*
  Setup SVC to reset value.
*/
__STATIC_INLINE void SVC_Setup(void)
{
#if ((__ARM_ARCH_7A__ == 0U) && (__ARM_ARCH_8A__ == 0U))
    /* Service Call interrupt might be configured before kernel start     */
    /* and when its priority is lower or equal to BASEPRI, svc intruction */
    /* causes a Hard Fault.                                               */
    NVIC_SetPriority(SVCall_IRQn, 0U);
#endif
}

/* Kernel initialization state */
static osKernelState_t KernelState = osKernelInactive;

osStatus_t osKernelInitialize(void)
{
    osStatus_t stat;

    if (IS_IRQ()) {
        stat = osErrorISR;
    } else {
        if (KernelState == osKernelInactive) {
#if defined(RTE_Compiler_EventRecorder)
            EvrFreeRTOSSetup(0U);
#endif
#if defined(RTE_RTOS_FreeRTOS_HEAP_5) && (HEAP_5_REGION_SETUP == 1)
            vPortDefineHeapRegions(configHEAP_5_REGIONS);
#endif
            KernelState = osKernelReady;
            stat = osOK;
        } else {
            stat = osError;
        }
    }

    return (stat);
}

osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
    if (version != NULL) {
        /* Version encoding is major.minor.rev: mmnnnrrrr dec */
        version->api = KERNEL_VERSION;
        version->kernel = KERNEL_VERSION;
    }

    if ((id_buf != NULL) && (id_size != 0U)) {
        if (id_size > sizeof(KERNEL_ID)) {
            id_size = sizeof(KERNEL_ID);
        }
        memcpy(id_buf, KERNEL_ID, id_size);
    }

    return (osOK);
}

osKernelState_t osKernelGetState(void)
{
    osKernelState_t state;

    switch (xTaskGetSchedulerState()) {
    case taskSCHEDULER_RUNNING:
        state = osKernelRunning;
        break;

    case taskSCHEDULER_SUSPENDED:
        state = osKernelLocked;
        break;

    case taskSCHEDULER_NOT_STARTED:
    default:
        if (KernelState == osKernelReady) {
            state = osKernelReady;
        } else {
            state = osKernelInactive;
        }
        break;
    }

    return (state);
}

osStatus_t osKernelStart(void)
{
    osStatus_t stat;

    if (IS_IRQ()) {
        stat = osErrorISR;
    } else {
        if (KernelState == osKernelReady) {
            /* Ensure SVC priority is at the reset value */
            SVC_Setup();
            /* Change state to enable IRQ masking check */
            KernelState = osKernelRunning;
            /* Start the kernel scheduler */
            vTaskStartScheduler();
            stat = osOK;
        } else {
            stat = osError;
        }
    }

    return (stat);
}

int32_t osKernelLock(void)
{
    int32_t lock;

    if (IS_IRQ()) {
        lock = (int32_t)osErrorISR;
    } else {
        switch (xTaskGetSchedulerState()) {
        case taskSCHEDULER_SUSPENDED:
            lock = 1;
            break;

        case taskSCHEDULER_RUNNING:
            vTaskSuspendAll();
            lock = 0;
            break;

        case taskSCHEDULER_NOT_STARTED:
        default:
            lock = (int32_t)osError;
            break;
        }
    }

    return (lock);
}

int32_t osKernelUnlock(void)
{
    int32_t lock;

    if (IS_IRQ()) {
        lock = (int32_t)osErrorISR;
    } else {
        switch (xTaskGetSchedulerState()) {
        case taskSCHEDULER_SUSPENDED:
            lock = 1;

            if (xTaskResumeAll() != pdTRUE) {
                if (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED) {
                    lock = (int32_t)osError;
                }
            }
            break;

        case taskSCHEDULER_RUNNING:
            lock = 0;
            break;

        case taskSCHEDULER_NOT_STARTED:
        default:
            lock = (int32_t)osError;
            break;
        }
    }

    return (lock);
}

int32_t osKernelRestoreLock(int32_t lock)
{
    if (IS_IRQ()) {
        lock = (int32_t)osErrorISR;
    } else {
        switch (xTaskGetSchedulerState()) {
        case taskSCHEDULER_SUSPENDED:
        case taskSCHEDULER_RUNNING:
            if (lock == 1) {
                vTaskSuspendAll();
            } else {
                if (lock != 0) {
                    lock = (int32_t)osError;
                } else {
                    if (xTaskResumeAll() != pdTRUE) {
                        if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
                            lock = (int32_t)osError;
                        }
                    }
                }
            }
            break;

        case taskSCHEDULER_NOT_STARTED:
        default:
            lock = (int32_t)osError;
            break;
        }
    }

    return (lock);
}

uint32_t osKernelGetTickCount(void)
{
    TickType_t ticks;

    if (IS_IRQ()) {
        ticks = xTaskGetTickCountFromISR();
    } else {
        ticks = xTaskGetTickCount();
    }

    return (ticks);
}

uint32_t osKernelGetTickFreq(void)
{
    return (configTICK_RATE_HZ);
}

/*---------------------------------------------------------------------------*/

osThreadId_t osThreadNew(
    osThreadFunc_t func,
    void *argument,
    const osThreadAttr_t *attr)
{
    const char *name;
    uint32_t stack;
    TaskHandle_t hTask;
    UBaseType_t prio;
    int32_t mem;

    hTask = NULL;

    if (!IS_IRQ() && (func != NULL)) {
        stack = configMINIMAL_STACK_SIZE;
        prio = (UBaseType_t)osPriorityNormal;

        name = NULL;
        mem = -1;

        if (attr != NULL) {
            if (attr->name != NULL) {
                name = attr->name;
            }
            if (attr->priority != osPriorityNone) {
                prio = (UBaseType_t)attr->priority;
            }

            if ((prio < osPriorityIdle) || (prio > osPriorityISR) ||
                ((attr->attr_bits & osThreadJoinable) == osThreadJoinable)) {
                return (NULL);
            }

            if (attr->stack_size > 0U) {
                /* In FreeRTOS stack is not in bytes,                        */
                /* but in sizeof(StackType_t) which is 4 on ARM ports.       */
                /* Stack size should be therefore 4 byte aligned in order to */
                /* avoid division caused side effects                        */
                stack = attr->stack_size / sizeof(StackType_t);
            }

            if ((attr->cb_mem != NULL) &&
                (attr->cb_size >= sizeof(StaticTask_t)) &&
                (attr->stack_mem != NULL) && (attr->stack_size > 0U)) {
                mem = 1;
            } else {
                if ((attr->cb_mem == NULL) && (attr->cb_size == 0U) &&
                    (attr->stack_mem == NULL)) {
                    mem = 0;
                }
            }
        } else {
            mem = 0;
        }

        if (mem == 1) {
            hTask = xTaskCreateStatic(
                (TaskFunction_t)func,
                name,
                stack,
                argument,
                prio,
                (StackType_t *)attr->stack_mem,
                (StaticTask_t *)attr->cb_mem);
        } else {
            if (mem == 0) {
                if (xTaskCreate(
                        (TaskFunction_t)func,
                        name,
                        (uint16_t)stack,
                        argument,
                        prio,
                        &hTask) != pdPASS) {
                    hTask = NULL;
                }
            }
        }
    }

    return ((osThreadId_t)hTask);
}

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    TaskHandle_t hTask = (TaskHandle_t)thread_id;
    uint32_t rflags;
    BaseType_t yield;

    if ((hTask == NULL) || ((flags & THREAD_FLAGS_INVALID_BITS) != 0U)) {
        rflags = (uint32_t)osErrorParameter;
    } else {
        rflags = (uint32_t)osError;

        if (IS_IRQ()) {
            yield = pdFALSE;

            (void)xTaskNotifyFromISR(hTask, flags, eSetBits, &yield);
            (void)xTaskNotifyAndQueryFromISR(
                hTask, 0, eNoAction, &rflags, NULL);

            portYIELD_FROM_ISR(yield);
        } else {
            (void)xTaskNotify(hTask, flags, eSetBits);
            (void)xTaskNotifyAndQuery(hTask, 0, eNoAction, &rflags);
        }
    }
    /* Return flags after setting */
    return (rflags);
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    TaskHandle_t hTask;
    uint32_t rflags, cflags;

    if (IS_IRQ()) {
        rflags = (uint32_t)osErrorISR;
    } else if ((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
        rflags = (uint32_t)osErrorParameter;
    } else {
        hTask = xTaskGetCurrentTaskHandle();

        if (xTaskNotifyAndQuery(hTask, 0, eNoAction, &cflags) == pdPASS) {
            rflags = cflags;
            cflags &= ~flags;

            if (xTaskNotify(hTask, cflags, eSetValueWithOverwrite) != pdPASS) {
                rflags = (uint32_t)osError;
            }
        } else {
            rflags = (uint32_t)osError;
        }
    }

    /* Return flags before clearing */
    return (rflags);
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    uint32_t rflags, nval;
    uint32_t clear;
    TickType_t t0, td, tout;
    BaseType_t rval;

    if (IS_IRQ()) {
        rflags = (uint32_t)osErrorISR;
    } else if ((flags & THREAD_FLAGS_INVALID_BITS) != 0U) {
        rflags = (uint32_t)osErrorParameter;
    } else {
        if ((options & osFlagsNoClear) == osFlagsNoClear) {
            clear = 0U;
        } else {
            clear = flags;
        }

        rflags = 0U;
        tout = timeout;

        t0 = xTaskGetTickCount();
        do {
            rval = xTaskNotifyWait(0, clear, &nval, tout);

            if (rval == pdPASS) {
                rflags &= flags;
                rflags |= nval;

                if ((options & osFlagsWaitAll) == osFlagsWaitAll) {
                    if ((flags & rflags) == flags) {
                        break;
                    } else {
                        if (timeout == 0U) {
                            rflags = (uint32_t)osErrorResource;
                            break;
                        }
                    }
                } else {
                    if ((flags & rflags) != 0) {
                        break;
                    } else {
                        if (timeout == 0U) {
                            rflags = (uint32_t)osErrorResource;
                            break;
                        }
                    }
                }

                /* Update timeout */
                td = xTaskGetTickCount() - t0;

                if (td > tout) {
                    tout = 0;
                } else {
                    tout -= td;
                }
            } else {
                if (timeout == 0) {
                    rflags = (uint32_t)osErrorResource;
                } else {
                    rflags = (uint32_t)osErrorTimeout;
                }
            }
        } while (rval != pdFAIL);
    }

    /* Return flags before clearing */
    return (rflags);
}

/*---------------------------------------------------------------------------*/

/* External Idle and Timer task static memory allocation functions */

/* External Idle and Timer task static memory allocation functions */
extern void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize);
extern void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize);

/* Idle task control block and stack */
static StaticTask_t Idle_TCB;
static StackType_t Idle_Stack[configMINIMAL_STACK_SIZE];

/* Timer task control block and stack */
static StaticTask_t Timer_TCB;
static StackType_t Timer_Stack[configTIMER_TASK_STACK_DEPTH];

/*
  vApplicationGetIdleTaskMemory gets called when
  configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &Idle_TCB;
    *ppxIdleTaskStackBuffer = &Idle_Stack[0];
    *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}

/*
  vApplicationGetTimerTaskMemory gets called when
  configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetTimerTaskMemory(
    StaticTask_t **ppxTimerTaskTCBBuffer,
    StackType_t **ppxTimerTaskStackBuffer,
    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &Timer_TCB;
    *ppxTimerTaskStackBuffer = &Timer_Stack[0];
    *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}
