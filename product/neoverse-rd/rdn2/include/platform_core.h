/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_CORE_H
#define PLATFORM_CORE_H

#include <fwk_assert.h>

#include <stdint.h>

#define PLATFORM_CORE_PER_CLUSTER_MAX 1

#define CORES_PER_CLUSTER 1
#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
#    define NUMBER_OF_CLUSTERS 16
#elif (PLATFORM_VARIANT == 1)
#    define NUMBER_OF_CLUSTERS 8
#elif (PLATFORM_VARIANT == 2)
#    define NUMBER_OF_CLUSTERS 4
#else
#    error "Unsupported PLATFORM_VARIANT value"
#endif

/* Number of chips supported on the platform. */
enum platform_chip_id {
    PLATFORM_CHIP_0,
#if (PLATFORM_VARIANT == 2)
    PLATFORM_CHIP_1,
    PLATFORM_CHIP_2,
    PLATFORM_CHIP_3,
#endif
    PLATFORM_CHIP_COUNT
};

#define MAX_PE_PER_CORE    1
#define MAX_PE_PER_CLUSTER (CORES_PER_CLUSTER * MAX_PE_PER_CORE)
#define MAX_PE_PER_CHIP    (NUMBER_OF_CLUSTERS * MAX_PE_PER_CLUSTER)

#define MPIDR_AFF0_SHIFT         (0U)
#define MPIDR_AFF1_SHIFT         (8U)
#define MPIDR_AFF2_SHIFT         (16U)
#define MPIDR_AFF3_SHIFT         (32U)
#define MPIDR_IMPL_MASK          (0xffU)
#define GET_AFF(shift, mpid_val) ((mpid_val >> shift) & MPIDR_IMPL_MASK)
#define GET_AFF0(mpid_val)       GET_AFF(MPIDR_AFF0_SHIFT, mpid_val)
#define GET_AFF1(mpid_val)       GET_AFF(MPIDR_AFF1_SHIFT, mpid_val)
#define GET_AFF2(mpid_val)       GET_AFF(MPIDR_AFF2_SHIFT, mpid_val)
#define GET_AFF3(mpid_val)       GET_AFF(MPIDR_AFF3_SHIFT, mpid_val)

static inline unsigned int platform_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

static inline unsigned int platform_get_core_per_cluster_count(
    unsigned int cluster)
{
    fwk_assert(cluster < platform_get_cluster_count());

    return CORES_PER_CLUSTER;
}

static inline unsigned int platform_get_core_count(void)
{
    return platform_get_core_per_cluster_count(0) *
        platform_get_cluster_count();
}

static inline uint8_t platform_calc_core_pos(uint64_t mpid)
{
    uint8_t chip_id;
    uint8_t cluster_id;
    uint8_t core_id;
    uint8_t thread_id;
    uint8_t position;

    chip_id = GET_AFF3(mpid);
    cluster_id = GET_AFF2(mpid);
    core_id = GET_AFF1(mpid);
    thread_id = GET_AFF0(mpid);

    position = (uint8_t)(
        (chip_id * MAX_PE_PER_CHIP) + (cluster_id * MAX_PE_PER_CLUSTER) +
        (core_id * MAX_PE_PER_CORE) + thread_id);

    return position;
}

#endif /* PLATFORM_CORE_H */
