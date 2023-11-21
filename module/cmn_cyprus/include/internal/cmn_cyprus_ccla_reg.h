/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for accessing CCG Link Agent (CCLA)
 *     registers.
 */

#ifndef CMN_CYPRUS_CCLA_REG_INTERNAL_H
#define CMN_CYPRUS_CCLA_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

#include <stdint.h>

/*
 * CCLA Upper Link Layer (ULL) Control Register bitfields.
 */
enum CCLA_ULL_CTL {
    /* VD Init message Send */
    CCLA_ULL_CTL_SEND_VD_INIT,

    /* ULL-to-ULL mode Enable */
    CCLA_ULL_CTL_ULL_TO_ULL_EN,

    /* CCLA ULL control register bitfield count */
    CCG_ULL_CTL_COUNT,
};

/*
 * CCLA Upper Link Layer (ULL) Status Register bitfields.
 */
enum CCLA_ULL_STATUS {
    /* Tx Stop/Run status */
    CCLA_ULL_STATUS_TX_ULL,

    /* Rx Stop/Run status */
    CCLA_ULL_STATUS_RX_ULL,

    /* CCLA ULL status register bitfield count */
    CCLA_ULL_STATUS_COUNT,
};

/*
 * Set CCLA ULL control register bitfield.
 *
 * \param ccla_reg Pointer to the CCLA node.
 *      \pre The CCLA node pointer must be valid.
 * \param bitfield CCLA ULL control register bitfield.
 *      \pre CCLA ULL control register bitfield must be valid.
 * \param value Value to be set in the CCLA ULL control register bitfield.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_SUPPORT Requested bitfield not supported.
 */
int ccla_set_ull_ctl(
    struct cmn_cyprus_ccla_reg *ccla_reg,
    enum CCLA_ULL_CTL bitfield,
    uint8_t value);

/*
 * Get CCLA ULL status register bitfield.
 *
 * \param ccla_reg Pointer to the CCLA node.
 *      \pre The CCLA node pointer must be valid.
 * \param bitfield CCLA ULL status register bitfield.
 *      \pre CCLA ULL status register bitfield must be valid.
 * \param[out] value Pointer to the variable that would store the value.
 *      \pre The pointer must be valid.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid value pointer.
 * \retval ::FWK_E_SUPPORT Requested bitfield not supported.
 */
int ccla_get_ull_status(
    struct cmn_cyprus_ccla_reg *ccla_reg,
    enum CCLA_ULL_STATUS bitfield,
    uint8_t *value);

#endif /* CMN_CYPRUS_CCLA_REG_INTERNAL_H */
