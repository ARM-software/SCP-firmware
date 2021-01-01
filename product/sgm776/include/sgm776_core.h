/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM776_CORE_H
#define SGM776_CORE_H

#define SGM776_CORE_PER_CLUSTER_MAX 8
#define SGM776_CLUSTER_COUNT 1

unsigned int sgm776_core_get_count(void);
unsigned int sgm776_cluster_get_count(void);

#endif /* SGM776_CORE_H */
