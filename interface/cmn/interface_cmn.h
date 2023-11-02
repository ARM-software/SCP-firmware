/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERFACE_CMN_H
#define INTERFACE_CMN_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupInterfaces Interfaces
 * @{
 */

/*!
 * \defgroup GroupCmn CMN module interface
 *
 * \brief Interface definition for CMN drivers.
 *
 * \details This provides an generic interface for the modules to bind to a
 *          platform specific CMN module.
 * @{
 */

/*!
 * \brief CMN interface to manage mappings in RN-SAM
 */
struct interface_cmn_memmap_rnsam_api {
    /*!
     * \brief Program or update the given IO memory carveout in the RN-SAM of
     *        all the nodes
     * \param base Base address of the carveout to be mapped
     * \param size Size of the carveout
     * \param node_id Target node id to which carveout will be mapped
     *
     * \retval ::FWK_SUCCESS on successfully mapping the region
     * \retval ::FWK_E_DATA if mapping region is invalid
     * \return One of the standard framework status codes
     */
    int (*map_io_region)(uint64_t base, size_t size, uint32_t node_id);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* INTERFACE_CMN_H */
