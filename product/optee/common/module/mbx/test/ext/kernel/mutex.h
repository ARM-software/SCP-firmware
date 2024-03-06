/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is a modified version of optee_os:core/include/kernel/mutex.h
 * It includes the bare minimum types and prototypes for testing purposes.
 */

#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

struct mutex {
};

void mutex_init(struct mutex *m);
void mutex_unlock(struct mutex *m);

#endif /*KERNEL_MUTEX_H*/
