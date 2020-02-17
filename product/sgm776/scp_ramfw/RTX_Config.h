/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      RTX2 v5 configuration file.
 *      The file must be called RTX_Config.h as it is included by an RTX file
 *      in order to create an object file containing the configuration.
 */

#ifndef RTX_CONFIG_H_
#define RTX_CONFIG_H_

/* System */
#define OS_DYNAMIC_MEM_SIZE        0
#define OS_TICK_FREQ               1000 /* Hz */
#define OS_ROBIN_ENABLE            0
#define OS_ROBIN_TIMEOUT           0
#define OS_ISR_FIFO_QUEUE          16

/* Thread */
#define OS_THREAD_OBJ_MEM          0
#define OS_THREAD_NUM              1
#define OS_THREAD_DEF_STACK_NUM    0
#define OS_THREAD_USER_STACK_SIZE  0
#define OS_STACK_SIZE              200
#define OS_IDLE_THREAD_STACK_SIZE  200
#define OS_STACK_CHECK             1
#define OS_STACK_WATERMARK         0
#define OS_PRIVILEGE_MODE          1

/* Timer */
#define OS_TIMER_OBJ_MEM           0
#define OS_TIMER_NUM               1
#define OS_TIMER_THREAD_PRIO       40
#define OS_TIMER_THREAD_STACK_SIZE 200
#define OS_TIMER_CB_QUEUE          4

/* Event flags */
#define OS_EVFLAGS_OBJ_MEM         0
#define OS_EVFLAGS_NUM             1

#define OS_MUTEX_OBJ_MEM           0
#define OS_MUTEX_NUM               1
#define OS_SEMAPHORE_OBJ_MEM       0
#define OS_SEMAPHORE_NUM           1
#define OS_MEMPOOL_OBJ_MEM         0
#define OS_MEMPOOL_NUM             1
#define OS_MEMPOOL_DATA_SIZE       0
#define OS_MSGQUEUE_OBJ_MEM        0
#define OS_MSGQUEUE_NUM            1
#define OS_MSGQUEUE_DATA_SIZE      0

#endif /* RTX_CONFIG_H_ */
