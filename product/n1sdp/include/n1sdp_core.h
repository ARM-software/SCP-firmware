/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_CORE_H
#define N1SDP_CORE_H

#include <stdint.h>

#define CORES_PER_CLUSTER  2
#define NUMBER_OF_CLUSTERS 2

unsigned int n1sdp_core_get_core_per_cluster_count(unsigned int cluster);
unsigned int n1sdp_core_get_core_count(void);
unsigned int n1sdp_core_get_cluster_count(void);

#endif /* N1SDP_CORE_H */
