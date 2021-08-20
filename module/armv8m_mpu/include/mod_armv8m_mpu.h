/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_ARMV8M_MPU_H
#define MOD_ARMV8M_MPU_H

#include <fmw_cmsis.h>

#include <stddef.h>

/*!
 * \ingroup GroupModules
 * \addtogroup GroupMPUARMv8M MPU (ARMv8-M)
 * \{
 */

/*!
 * \brief MPU_MAIR registers indices.
 */
enum mod_armv8m_mpu_attr_id {
    MPU_ATTR_0,
    MPU_ATTR_1,
    MPU_ATTR_2,
    MPU_ATTR_3,
    MPU_ATTR_4,
    MPU_ATTR_5,
    MPU_ATTR_6,
    MPU_ATTR_7,
    MPU_MAX_ATTR_COUNT,
};

/*!
 * \brief Module configuration.
 */
struct mod_armv8m_mpu_config {
    /*!
     * \brief Number of MPU attributes.
     */
    uint8_t attributes_count;

    /*!
     * \brief Pointer to array of MPU attributes.
     */
    const uint8_t *attributes;

    /*!
     * \brief First region number.
     */
    uint32_t first_region_number;

    /*!
     * \brief Number of MPU regions.
     */
    uint32_t region_count;

    /*!
     * \brief Pointer to array of MPU regions.
     *
     * \details Documentation for the \c ARM_MPU_Region_t can be found in the
     *      CMSIS 5 documentation for the ARMv8-M MPU.
     *
     * \see http://arm-software.github.io/CMSIS_5/General/html/index.html
     */
    const ARM_MPU_Region_t *regions;
};

/*!
 * \}
 */

#endif /* MOD_ARMV8M_MPU_H */
