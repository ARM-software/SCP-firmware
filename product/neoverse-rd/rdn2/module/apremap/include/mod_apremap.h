/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Application Processor address remap module for SGI/RD platforms.
 */

#ifndef MOD_APREMAP_H
#define MOD_APREMAP_H

#include <interface_address_remapper.h>

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupApRemap AP Remap
 *
 * \brief HAL Module used to remap the Application Processor's address space to
 *      MSCP's address space.
 *
 * \details This module uses the AP remap address translation registers to remap
 *      the AP address space to MSCP address space including mapping the CMN
 *      configuration region in AP address space to MSCP address space.
 *      \{
 */

/*!
 * \brief AP remap register
 */
struct mod_apremap_reg {
    /*! Address Translation register for access AP address space. */
    FWK_RW uint32_t ADDR_TRANS;
    /*! Address Translation register for access debug components. */
    FWK_RW uint32_t DBG_ADDR_TRANS;
};

/*!
 * \brief Module configuration.
 */
struct mod_apremap_config {
    /*! Base address of the address translation registers. */
    uintptr_t base;
};

/*!
 * \brief API to enable/disable the CMN address translation.
 */
struct mod_apremap_cmn_atrans_api {
    /*!
     * \brief API to enable CMN address translation
     *
     * \details Enables CMN Address Translation to translate MSCP address
     * `0x6000_0000 - 0x9FFF_FFFFF` to `(4TB * CHIPID) + (CMN register offset)`
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     */
    int (*enable)(void);

    /*!
     * \brief API to disable CMN address translation
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     */
    int (*disable)(void);
};

/*!
 * \brief Module API indicies.
 */
enum mod_apremap_api_idx {
    MOD_APREMAP_API_IDX_AP_MEM_RW,
    MOD_APREMAP_API_IDX_CMN_ATRANS,
    MOD_APREMAP_API_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_APREMAP_H */
