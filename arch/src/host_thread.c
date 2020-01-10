/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CMSIS-RTOS2 functions are mapped to POSIX functions to run on the host.
 */

#include <cmsis_os2.h>
#include <pthread.h>
#include <unistd.h>

#include <fwk_assert.h>
#include <fwk_mm.h>

#include <stdbool.h>

/* CMSIS-RTOS2 thread context data */
struct thread_data {
    /* Function associated with a thread */
    osThreadFunc_t func;
    /* Argument of the function associated with a thread */
    void *arg;
    /* Condition associated with the thread */
    pthread_cond_t cond;
    /* Indicate the flags set for the thread */
    uint32_t signal_flags;
};

/* Each pthread owns an associatied struct thread_data */
static _Thread_local struct thread_data *local_thread_data;

/*
 * Mutex used to block the launch of the threads while the main thread is
 * initializing the framework and ensure there is only one thread that is
 * launched at a time.
 */
static pthread_mutex_t mutex;

/*
 * pthread_func is a pthread compatible wrapper for the CMSIS-RTOS2 thread
 * function used for:
 * 1) This function's signature is compatible with the pthread_create(). The
 *    actual thread function is called internally and has a different prototype
 *    as per CMSIS-RTOS2 API.
 * 2) This function adds a barrier mechanism to ensure the thread is not run
 *    until osThreadStart() is invoked as expected by the CMSIS-RTOS2 API.
 * 3) Initialize local_thread_data when the thread runs for the first time.
 */
static void *pthread_func(void *data)
{
    int status;

    local_thread_data = data;

    /*
     * The mutex associated with the condition needs to be locked before
     * pthread_cond_wait is called. This mutex is unlocked by pthread_cond_wait
     * when the thread waits, and is locked again by the thread which wakes
     * up.
     */
    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        return NULL;

    /*
     * Run the thread's function. This function must not return.
     */
    local_thread_data->func(local_thread_data->arg);

    fwk_unreachable();

    return NULL;
}

osStatus_t osKernelInitialize(void)
{
    int status;

    status = pthread_mutex_init(&mutex, NULL);
    if (status != 0)
        return osError;

    /*
     * The main thread locks the mutex to ensure that no threads begin
     * execution before framework has completed initialization.
     */
    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        return osError;

    return osOK;
}

osStatus_t osKernelStart(void)
{
    int status;

    /*
     * The main thread releases the mutex to allow other threads to execute.
     */
    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        return osError;

    pause();

    fwk_unreachable();

    return osError;
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    int status;
    bool wait;
    uint32_t tmp_signal_flags;

    assert(flags != 0);
    assert(timeout == osWaitForever);

    while (true) {
        switch (options) {
        case osFlagsWaitAny:
            wait = (local_thread_data->signal_flags & flags) == 0;
            break;
        case osFlagsWaitAll:
            wait = (local_thread_data->signal_flags & flags) != flags;
            break;
        default:
            assert(false);
        }

        if (!wait)
            break;

        status = pthread_cond_wait(&local_thread_data->cond, &mutex);
        if (status != 0)
            return osFlagsErrorUnknown;
    }

    tmp_signal_flags = local_thread_data->signal_flags;

    if (!(options & osFlagsNoClear))
        local_thread_data->signal_flags &= ~flags;

    return tmp_signal_flags;
}

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    int status;
    struct thread_data *thread_data = (struct thread_data *)thread_id;

    thread_data->signal_flags |= flags;

    /*
     * The thread signals the condition but the thread waiting cannot wake
     * up until this thread calls pthread_cond_wait as it owns the mutex.
     */
    status = pthread_cond_signal(&thread_data->cond);
    if (status != 0)
        return osFlagsErrorUnknown;

    return flags;
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    uint32_t old_flags = local_thread_data->signal_flags;

    local_thread_data->signal_flags &= ~flags;

    return old_flags;
}

osThreadId_t osThreadNew(osThreadFunc_t func, void *argument,
    const osThreadAttr_t *attr)
{
    int status;
    struct thread_data *thread_data;
    pthread_t *pthread_id;

    thread_data = fwk_mm_calloc(1, sizeof(struct thread_data));

    pthread_id = fwk_mm_calloc(1, sizeof(pthread_t));

    status = pthread_cond_init(&thread_data->cond, NULL);
    if (status != 0)
        return NULL;

    thread_data->func = func;
    thread_data->arg = argument;
    thread_data->signal_flags = 0;

    status = pthread_create(pthread_id, NULL, pthread_func, thread_data);
    if (status != 0)
        return NULL;

    return thread_data;
}
