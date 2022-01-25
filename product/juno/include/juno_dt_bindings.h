/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_DT_BINDINGS_H
#define JUNO_DT_BINDINGS_H

#include "juno_mmap.h"

/* Level 0 */
#define POWER_DOMAIN_IDX_BIG_CPU0       0
#define POWER_DOMAIN_IDX_BIG_CPU1       1
#define POWER_DOMAIN_IDX_LITTLE_CPU0    2
#define POWER_DOMAIN_IDX_LITTLE_CPU1    3
#define POWER_DOMAIN_IDX_LITTLE_CPU2    4
#define POWER_DOMAIN_IDX_LITTLE_CPU3    5

    /* Level 1 */
#define POWER_DOMAIN_IDX_BIG_SSTOP      6
#define POWER_DOMAIN_IDX_LITTLE_SSTOP   7
#define POWER_DOMAIN_IDX_DBGSYS         8
#define POWER_DOMAIN_IDX_GPUTOP         9

    /* Level 2 */
#define POWER_DOMAIN_IDX_SYSTOP         10

#define POWER_DOMAIN_IDX_NONE           -1


#endif /* JUNO_DT_BINDINGS_H */
