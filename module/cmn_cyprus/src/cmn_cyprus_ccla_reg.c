/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing CCLA registers.
 */

#include <internal/cmn_cyprus_ccla_reg.h>
#include <internal/cmn_cyprus_reg.h>

#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/* CCLA ULL CTL register bitfield */
#define ULL_CTL_ULL_TO_ULL_EN_POS  1
#define ULL_CTL_ULL_TO_ULL_EN_MASK UINT64_C(0x1)
#define ULL_CTL_SEND_VD_INIT_POS   0
#define ULL_CTL_SEND_VD_INIT_MASK  UINT64_C(0x1)

/* CCLA ULL STATUS register bitfield */
#define ULL_STATUS_TX_ULL_STATE_POS  0
#define ULL_STATUS_TX_ULL_STATE_MASK UINT64_C(0x1)
#define ULL_STATUS_RX_ULL_STATE_POS  1
#define ULL_STATUS_RX_ULL_STATE_MASK UINT64_C(0x1)

/* Set the given bitfield in the CCLA Upper Link Layer (ULL) Control register */
int ccla_set_ull_ctl(
    struct cmn_cyprus_ccla_reg *ccla_reg,
    enum CCLA_ULL_CTL bitfield,
    uint8_t value)
{
    int status;
    status = FWK_SUCCESS;

    switch (bitfield) {
    case CCLA_ULL_CTL_SEND_VD_INIT:
        /* Send VD Init message when ULL-to-ULL mode is enabled */
        ccla_reg->CCLA_ULL_CTL |=
            ((value & ULL_CTL_SEND_VD_INIT_MASK) << ULL_CTL_SEND_VD_INIT_POS);
        break;

    case CCLA_ULL_CTL_ULL_TO_ULL_EN:
        /* Enable ULL-to-ULL mode */
        ccla_reg->CCLA_ULL_CTL |=
            ((value & ULL_CTL_ULL_TO_ULL_EN_MASK) << ULL_CTL_ULL_TO_ULL_EN_POS);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

/* Get the given bitfield in the CCLA Upper Link Layer (ULL) Status register */
int ccla_get_ull_status(
    struct cmn_cyprus_ccla_reg *ccla_reg,
    enum CCLA_ULL_STATUS bitfield,
    uint8_t *value)
{
    int status;
    uint64_t reg_value;

    if (value == NULL) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;
    reg_value = ccla_reg->CCLA_ULL_STATUS;

    switch (bitfield) {
    case CCLA_ULL_STATUS_TX_ULL:
        /* Get Tx ULL state */
        *value =
            ((reg_value >> ULL_STATUS_TX_ULL_STATE_POS) &
             ULL_STATUS_TX_ULL_STATE_MASK);
        break;

    case CCLA_ULL_STATUS_RX_ULL:
        /* Get Rx ULL state */
        *value =
            ((reg_value >> ULL_STATUS_RX_ULL_STATE_POS) &
             ULL_STATUS_RX_ULL_STATE_MASK);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}
