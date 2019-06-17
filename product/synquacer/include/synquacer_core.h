/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_CORE_H
#define SYNQUACER_CORE_H

#include <stdint.h>

/* Maximum number of clusters */
#define SYNQUACER_CSS_CPUS_CLUSTER_MAX UINT32_C(12)

/* Maximum number of CPUs per cluster */
#define SYNQUACER_CSS_CPUS_PER_CLUSTER_MAX UINT32_C(2)

/* Maximum number of CPUs */
#define SYNQUACER_CSS_CPUS_MAX \
    (SYNQUACER_CSS_CPUS_CLUSTER_MAX * SYNQUACER_CSS_CPUS_PER_CLUSTER_MAX)

uint32_t synquacer_core_get_core_count(void);
uint32_t synquacer_core_get_cluster_count(void);
uint32_t synquacer_core_get_core_per_cluster_count(void);

#endif /* SYNQUACER_CORE_H */
