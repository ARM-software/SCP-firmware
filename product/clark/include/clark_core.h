/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLARK_CORE_H
#define CLARK_CORE_H

#define CLARK_CORE_PER_CLUSTER_MAX 8

unsigned int clark_core_get_core_per_cluster_count(unsigned int cluster);
unsigned int clark_core_get_core_count(void);
unsigned int clark_core_get_cluster_count(void);

#endif /* CLARK_CORE_H */
