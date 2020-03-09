/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_CORE_H
#define N1SDP_CORE_H

#include <stdbool.h>
#include <stdint.h>

#define CORES_PER_CLUSTER  2
#define NUMBER_OF_CLUSTERS 2

unsigned int n1sdp_core_get_core_per_cluster_count(unsigned int cluster);
unsigned int n1sdp_core_get_core_count(void);
unsigned int n1sdp_core_get_cluster_count(void);
bool n1sdp_is_multichip_enabled(void);
uint8_t n1sdp_get_chipid(void);

#endif /* N1SDP_CORE_H */
