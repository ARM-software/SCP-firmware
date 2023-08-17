/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO SCP to PCC communication protocol module.
 */

#ifndef MOD_MORELLO_SCP2PCC_H
#define MOD_MORELLO_SCP2PCC_H

#include <stdbool.h>
#include <stdint.h>
/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLOScp2pcc MORELLO SCP2PCC communication protocol
 *
 * \brief Driver support for MORELLO SCP2PCC.
 *
 * \details This module provides support for SCP to PCC communication.
 *
 * \{
 */

#define MORELLO_SCP2PCC_PCC_FW_VERSION_LEN         (3)
#define MORELLO_SCP2PCC_PCC_FW_VERSION_UPPER_INDEX (2)
#define MORELLO_SCP2PCC_PCC_FW_VERSION_MID_INDEX   (1)
#define MORELLO_SCP2PCC_PCC_FW_VERSION_LOWER_INDEX (0)

#define MORELLO_SCP2PCC_MCC_FW_VERSION_LEN         (3)
#define MORELLO_SCP2PCC_MCC_FW_VERSION_UPPER_INDEX (2)
#define MORELLO_SCP2PCC_MCC_FW_VERSION_MID_INDEX   (1)
#define MORELLO_SCP2PCC_MCC_FW_VERSION_LOWER_INDEX (0)

#define MORELLO_SCP2PCC_BOARD_SERIAL_NUM_LEN (16)

#define MORELLO_SCP2PCC_SILICON_REVISION_LEN     (2)
#define MORELLO_SCP2PCC_SILICON_REVISION_P_INDEX (0)
#define MORELLO_SCP2PCC_SILICON_REVISION_R_INDEX (1)

enum scp2pcc_msg_type {
    MOD_SCP2PCC_SEND_SHUTDOWN = 1,
    MOD_SCP2PCC_SEND_REBOOT,
    MOD_SCP2PCC_GET_PCC_FW_VERSION,
    MOD_SCP2PCC_GET_MCC_FW_VERSION,
    MOD_SCP2PCC_GET_BOARD_SERIAL_NUM,
    MOD_SCP2PCC_SET_SOC_FAN_SPEED,
    MOD_SCP2PCC_SET_CASE_FAN_SPEED,
    MOD_SCP2PCC_GET_SILICON_REVISION,
    MOD_SCP2PCC_MSG_COUNT,
};

enum scp2pcc_msg_status {
    SCP2PCC_MSG_STATUS_INVALID = 0,
    SCP2PCC_MSG_STATUS_SUCCESS,
    SCP2PCC_MSG_STATUS_FAIL,
};

/*!
 * \brief API to transfer data between SCP and PCC.
 */
struct mod_morello_scp2pcc_api {
    /*!
     * \brief Function to send request to PCC.
     *
     * \param type          SCP to PCC message type
     * \param req_data      SCP to PCC message request data
     * \param req_size      SCP to PCC message request data length
     * \param resp_data     SCP to PCC message response data
     * \param resp_size     SCP to PCC message response data length
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_TIMEOUT Operation timed out.
     * \retval ::FWK_E_DEVICE Operation at PCC failed.
     */
    int (*send)(
        enum scp2pcc_msg_type type,
        void *req_data,
        uint16_t req_size,
        void *resp_data,
        uint16_t *resp_size);
};
/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_SCP2PCC_H */
