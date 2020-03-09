/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CCN512_H
#define MOD_CCN512_H

#include <internal/ccn512.h>

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupCCN512 CCN512 Driver
 *
 * \brief CCN512 device driver.
 *
 * \details This module implements a device driver for the CCN512
 *
 * @{
 */

/*
 * \brief CCN512 memory map
 */
typedef struct {
    /*!
     * \cond
     * @{
     */
    ccn5xx_mn_reg_t MN_ID_34;
    ccn5xx_dbg_reg_t DEBUG_ID_34;
    ccn5xx_region_t RESERVED0[6];
    ccn5xx_hni_reg_t HNI_ID_13;
    ccn5xx_hni_reg_t HNI_ID_34;
    ccn5xx_region_t RESERVED1[6];
    ccn5xx_sbsx_reg_t SBSX_ID_3;
    ccn5xx_sbsx_reg_t SBSX_ID_8;
    ccn5xx_sbsx_reg_t SBSX_ID_21;
    ccn5xx_sbsx_reg_t SBSX_ID_26;
    ccn5xx_region_t RESERVED2[12];
    ccn5xx_hnf_reg_t HNF_ID_2;
    ccn5xx_hnf_reg_t HNF_ID_5;
    ccn5xx_hnf_reg_t HNF_ID_6;
    ccn5xx_hnf_reg_t HNF_ID_9;
    ccn5xx_hnf_reg_t HNF_ID_20;
    ccn5xx_hnf_reg_t HNF_ID_23;
    ccn5xx_hnf_reg_t HNF_ID_24;
    ccn5xx_hnf_reg_t HNF_ID_27;
    ccn5xx_region_t RESERVED3[24];
    ccn5xx_xp_reg_t XP_ID_0;
    ccn5xx_xp_reg_t XP_ID_1;
    ccn5xx_xp_reg_t XP_ID_2;
    ccn5xx_xp_reg_t XP_ID_3;
    ccn5xx_xp_reg_t XP_ID_4;
    ccn5xx_xp_reg_t XP_ID_5;
    ccn5xx_xp_reg_t XP_ID_6;
    ccn5xx_xp_reg_t XP_ID_7;
    ccn5xx_xp_reg_t XP_ID_8;
    ccn5xx_xp_reg_t XP_ID_9;
    ccn5xx_xp_reg_t XP_ID_10;
    ccn5xx_xp_reg_t XP_ID_11;
    ccn5xx_xp_reg_t XP_ID_12;
    ccn5xx_xp_reg_t XP_ID_13;
    ccn5xx_xp_reg_t XP_ID_14;
    ccn5xx_xp_reg_t XP_ID_15;
    ccn5xx_xp_reg_t XP_ID_16;
    ccn5xx_xp_reg_t XP_ID_17;
    ccn5xx_region_t RESERVED4[47];
    ccn5xx_rni_reg_t RNI_ID_1;
    ccn5xx_region_t RESERVED5[2];
    ccn5xx_rni_reg_t RNI_ID_4;
    ccn5xx_region_t RESERVED6[2];
    ccn5xx_rni_reg_t RNI_ID_7;
    ccn5xx_region_t RESERVED7[2];
    ccn5xx_rni_reg_t RNI_ID_10;
    ccn5xx_region_t RESERVED8[8];
    ccn5xx_rni_reg_t RNI_ID_19;
    ccn5xx_region_t RESERVED9[2];
    ccn5xx_rni_reg_t RNI_ID_22;
    ccn5xx_region_t RESERVED10[2];
    ccn5xx_rni_reg_t RNI_ID_25;
    ccn5xx_region_t RESERVED11[2];
    ccn5xx_rni_reg_t RNI_ID_28;
    /*!
    * \endcond
    * @}
    */
} ccn512_reg_t;

/*!
 * \brief APIs to configure ccn512.
 */
struct mod_ccn512_api {
    /*!
     * \brief APIs to configure ccn512
     *
     * \param none
     *
     */
    void (*ccn512_exit)(void);
};

/*!
 * \brief CCN512 device configuration data.
 */
struct mod_ccn512_module_config {
    /*! Base address of the device registers */
    ccn512_reg_t *reg_base;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CCN512_H */
