/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_CORE_H
#define MORELLO_CORE_H

#include <stdbool.h>
#include <stdint.h>

#define CORES_PER_CLUSTER 2
#define NUMBER_OF_CLUSTERS 2

unsigned int morello_core_get_core_per_cluster_count(unsigned int cluster);
unsigned int morello_core_get_core_count(void);
unsigned int morello_core_get_cluster_count(void);
bool morello_is_multichip_enabled(void);
uint8_t morello_get_chipid(void);

#endif /* MORELLO_CORE_H */
