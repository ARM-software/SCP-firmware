/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Platform System Support
 */

#ifndef MOD_PLATFORM_SYSTEM_H
#define MOD_PLATFORM_SYSTEM_H

#include <mod_power_domain.h>

#define WARM_RESET_MAX_RETRIES 10

/*!
 * \addtogroup GroupPLATFORMModule PLATFORM Product Modules
 * @{
 */

/*!
 * \defgroup GroupPLATFORMSystem PLATFORM System Support
 * @{
 */

/*!
 * \brief Additional PLATFORM system power states.
 */
enum mod_platform_system_power_states {
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP1,
    MOD_PLATFORM_SYSTEM_POWER_STATE_COUNT
};

/*!
 * \brief System power state masks.
 */
enum mod_platform_system_power_state_masks {
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP0_MASK =
        (1 << MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP0),
    MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP1_MASK =
        (1 << MOD_PLATFORM_SYSTEM_POWER_STATE_SLEEP1),
};

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_platform_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_PLATFORM_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of exposed interfaces */
    MOD_PLATFORM_SYSTEM_API_COUNT
};

/*!
 * \brief Events used by platform system module.
 */
enum mod_platform_system_event_idx {
    /*! Event requesting check for power domain OFF */
    MOD_PLATFORM_SYSTEM_CHECK_PD_OFF,

    /*! Number of defined events */
    MOD_PLATFORM_SYSTEM_EVENT_COUNT
};

/*!
 * \brief Event to check all CPUs are powered off.
 */
static const fwk_id_t mod_platform_system_event_check_ppu_off = FWK_ID_EVENT(
    FWK_MODULE_IDX_PLATFORM_SYSTEM,
    MOD_PLATFORM_SYSTEM_CHECK_PD_OFF);

/*!
 * \brief List of isolated CPU MPIDs.
 */
struct mod_platform_isolated_cpu_info {
    /*! Number of isolated CPUs */
    uint64_t isolated_cpu_count;

    /*!
     * MPID of Isolated CPUs represented as a list. Value of each MPID
     * specifies the affinity values as per by the MPIDR register format
     *    Bits 63:40 - should be zero
     *    Bits 39:32 - Affinity level 3
     *    Bits 31:24 - should be zero
     *    Bits 23:16 - Affinity level 2
     *    Bits 15:8  - Affinity level 1
     *    Bits 7:0   - Affinity level 0
     */
    uint64_t *isolated_cpu_mpid_list;
};

/*!
 * \brief Module configuration.
 */
struct mod_platform_system_config {
    /*! MPID number of the CPU to be used as primary CPU */
    uint64_t primary_cpu_mpid;

    /*! List of isolated CPUs MPID. */
    struct mod_platform_isolated_cpu_info isolated_cpu_info;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_PLATFORM_SYSTEM_H */
