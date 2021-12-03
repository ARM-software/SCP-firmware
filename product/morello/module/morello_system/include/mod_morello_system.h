/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO System Support
 */

#ifndef MOD_MORELLO_SYSTEM_H
#define MOD_MORELLO_SYSTEM_H

/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLOSystem MORELLO System Support
 * \{
 */

/*!
 * \brief MORELLO system macro definitions.
 */

/*! Offset of the Trusted SRAM between AP and SCP Address Space*/
#define AP_SCP_SRAM_OFFSET UINT32_C(0xA0000000)

/*! Address translation enable bit */
#define ADDR_TRANS_EN UINT32_C(0x1)


/*!
 * Offset of NIC-400 security 0 register for
 * non-secure CoreSight debug access
 */
#define NIC_400_SEC_0_CSAPBM_OFFSET UINT32_C(0x2A10001C)

/*!
 * \brief API indices.
 */
enum mod_morello_system_api_idx {
    /*! API index for the driver interface of the system_power module */
    MOD_MORELLO_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! API index for AP memory access */
    MOD_MORELLO_SYSTEM_API_IDX_AP_MEMORY_ACCESS,

    /*! API index for getting chip information */
    MOD_MORELLO_SYSTEM_API_IDX_CHIPINFO,

    /*! Number of exposed interfaces */
    MOD_MORELLO_SYSTEM_API_COUNT,
};

/*!
 * \brief APIs to enable/disable access AP memory in 1MB window.
 */
struct mod_morello_system_ap_memory_access_api {
    /*!
     * \brief Pointer to function that enables windowed access to AP memory.
     *
     * \param addr Base address of the 1MB AP memory window.
     *
     * \retval void.
     */
    void (*enable_ap_memory_access)(uint32_t addr);

    /*!
     * \brief Pointer to function that disables windowed access to AP memory.
     *
     * \retval void.
     */
    void (*disable_ap_memory_access)(void);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_SYSTEM_H */
