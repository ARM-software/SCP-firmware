/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP SCP to PCC communication protocol module.
 */

#ifndef MOD_N1SDP_SCP2PCC_H
#define MOD_N1SDP_SCP2PCC_H

#include <internal/n1sdp_scp2pcc.h>

#include <stdint.h>
/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPScp2pcc N1SDP SCP2PCC communication protocol
 *
 * \brief Driver support for N1SDP SCP2PCC.
 *
 * \details This module provides support for SCP to PCC communication.
 *
 * \{
 */

/*!
 * \brief N1SDP SCP to PCC module configuration
 */
struct mem_msg_config_st {
    /*! Pointer to memory location set at init. */
    unsigned volatile int *shared_alive_address;
    /*! Pointer to TX buffer shared between SCP and PCC */
    uintptr_t shared_tx_buffer;
    /*! Number of TX packets allocated */
    unsigned int shared_num_tx;
    /*! Pointer to RX buffer shared between SCP and PCC */
    uintptr_t shared_rx_buffer;
    /*! Number of RX packets allocated */
    unsigned int shared_num_rx;
};

/*!
 * \brief API to transfer data between scp and pcc.
 */
struct mod_n1sdp_scp2pcc_api {
    /*!
     * \brief Function to send data from SCP to PCC.
     *
     * \param data Data payload.
     * \param size Size of the payload to be sent.
     * \param type Indicates the type of payload sent.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*send)(void *data, uint16_t size, uint16_t type);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_SCP2PCC_H */
