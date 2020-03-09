/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_ARMV7M_MPU_H
#define MOD_ARMV7M_MPU_H

#include <fmw_cmsis.h>

#include <stddef.h>

/*!
 * \ingroup GroupModules
 * \addtogroup GroupMPUARMv7M MPU (ARMv7-M)
 * \{
 */

/*!
 * \brief Module configuration.
 */
struct mod_armv7m_mpu_config {
    /*!
     * \brief Number of MPU regions.
     */
    size_t region_count;

    /*!
     * \brief Pointer to array of MPU regions.
     *
     * \details Documentation for the \c ARM_MPU_Region_t can be found in the
     *      CMSIS 5 documentation for the ARMv7-M MPU.
     *
     * \see http://arm-software.github.io/CMSIS_5/General/html/index.html
     */
    const ARM_MPU_Region_t *regions;
};

/*!
 * \}
 */

#endif /* MOD_ARMV7M_MPU_H */
