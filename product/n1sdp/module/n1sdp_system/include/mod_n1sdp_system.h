/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP System Support
 */

#ifndef MOD_N1SDP_SYSTEM_H
#define MOD_N1SDP_SYSTEM_H

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPSystem N1SDP System Support
 * @{
 */

/*!
 * \brief N1SDP system macro definitions.
 */

/*!
 * N1SDP AP cores do not have internal ROM memory to boot code once they are
 * released from reset. It is the responsibility of SCP to copy the AP
 * firmware to internal/external memory and set the RVBAR register of the
 * AP cores to corresponding memory's base address and then switch ON
 * the PPU of primary core to release from reset. This macro specifies the
 * base address of the Trusted AP SRAM to which AP firmware will be copied
 * to and therefore the value to set in the RVBAR of all AP cores.
 */

/*! Offset of the Trusted SRAM between AP and SCP Address Space*/
#define AP_SCP_SRAM_OFFSET            UINT32_C(0xA0000000)

/*! AP Cores Reset Address in SCP Address Space */
#define AP_CORE_RESET_ADDR            UINT64_C(0xA4040000)

/*! Address translation enable bit */
#define ADDR_TRANS_EN                 UINT32_C(0x1)

/*! Source address of BL33 image to be used by BL31 */
#define BL33_SRC_BASE_ADDR            UINT32_C(0x14200000)

/*! Destination address of BL33 image to be used by BL31 */
#define BL33_DST_BASE_ADDR            UINT32_C(0xE0000000)

/*! Size of BL33 image to be used by BL31 */
#define BL33_SIZE                     UINT32_C(0x00200000)

/*!
 * Offset of NIC-400 security 0 register for
 * non-secure CoreSight debug access
 */
#define NIC_400_SEC_0_CSAPBM_OFFSET   UINT32_C(0x2A10001C)

/*!
 * \brief API indices.
 */
enum mod_n1sdp_system_api_idx {
    /*! API index for the driver interface of the system_power module */
    MOD_N1SDP_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! API index for AP memory access */
    MOD_N1SDP_SYSTEM_API_IDX_AP_MEMORY_ACCESS,

    /*! Number of exposed interfaces */
    MOD_N1SDP_SYSTEM_API_COUNT,
};

/*!
 * \brief APIs to enable/disable access AP memory in 1MB window.
 */
struct mod_n1sdp_system_ap_memory_access_api {
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
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_SYSTEM_H */
