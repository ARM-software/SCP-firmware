/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP SCP to SCP I2C communication protocol module.
 */

#ifndef MOD_N1SDP_C2C_I2C_H
#define MOD_N1SDP_C2C_I2C_H

#include <mod_power_domain.h>

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * \{
 */

/*!
 * \defgroup GroupN1SDPN1SDPC2C N1SDP SCP2SCP I2C communication protocol
 *
 * \brief Driver support for N1SDP C2C.
 *
 * \details This module provides support for SCP to SCP I2C communication.
 *
 * \{
 */

/*!
 * \brief N1SDP C2C Handshake commands
 */
enum n1sdp_c2c_cmd {
    N1SDP_C2C_CMD_CHECK_SECONDARY,
    N1SDP_C2C_CMD_PCIE_POWER_ON,
    N1SDP_C2C_CMD_PCIE_PHY_INIT,
    N1SDP_C2C_CMD_PCIE_CTRL_INIT,
    N1SDP_C2C_CMD_PCIE_LINK_TRAIN,
    N1SDP_C2C_CMD_PCIE_RC_SETUP,
    N1SDP_C2C_CMD_PCIE_VC1_CONFIG,
    N1SDP_C2C_CMD_PCIE_CCIX_CONFIG,
    N1SDP_C2C_CMD_CMN600_SET_CONFIG,
    N1SDP_C2C_CMD_CMN600_XCHANGE_CREDITS,
    N1SDP_C2C_CMD_CMN600_ENTER_SYS_COHERENCY,
    N1SDP_C2C_CMD_CMN600_ENTER_DVM_DOMAIN,
    N1SDP_C2C_CMD_GET_SLV_DDR_SIZE,
    N1SDP_C2C_CMD_TIMER_SYNC,
    N1SDP_C2C_CMD_POWER_DOMAIN_ON,
    N1SDP_C2C_CMD_POWER_DOMAIN_OFF,
    N1SDP_C2C_CMD_POWER_DOMAIN_GET_STATE,
    N1SDP_C2C_CMD_SHUTDOWN_OR_REBOOT,
};

/*!
 * \brief N1SDP SCP to SCP I2C module configuration
 */
struct n1sdp_c2c_dev_config {
    /*! Identifier of I2C device ID */
    fwk_id_t i2c_id;
    /*! I2C target address to be used */
    uint8_t target_addr;
    /*! PCIe element identifier for CCIX */
    fwk_id_t ccix_id;
};

/*!
 * \brief Module API indices
 */
enum n1sdp_c2c_api_idx {
    /*! Index of the N1SDP C2C secondary information API */
    N1SDP_C2C_API_IDX_SECONDARY_INFO,

    /*! Index of the N1SDP C2C power domain API */
    N1SDP_C2C_API_IDX_PD,

    /*! Number of APIs */
    N1SDP_C2C_API_COUNT
};

/*!
 * \brief N1SDP C2C secondary information API
 */
struct n1sdp_c2c_secondary_info_api {
    /*!
     * \brief API to check if secondary is alive or not.
     *
     * \retval true If secondary is alive.
     * \return false If secondary is not alive.
     */
    bool (*is_secondary_alive)(void);
    /*!
     * \brief API to get secondary chip's DDR size in GB.
     *
     * \param size_gb Pointer to storage where the size is stored.
     *
     * \retval ::FWK_SUCCESS If operation succeeds.
     * \return One of the possible error return codes.
     */
    int (*get_ddr_size_gb)(uint8_t *size_gb);
};

/*!
 * \brief N1SDP C2C power domain API
 */
struct n1sdp_c2c_pd_api {
    /*!
     * \brief API to set a power state in remote chip.
     *
     * \param cmd The C2C command type to issue.
     * \param pd_id The secondary chip's power domain ID.
     * \param pd_type The secondary chip's power domain type.
     *
     * \retval ::FWK_SUCCESS If operation succeeds.
     * \return One of the possible error return codes.
     */
    int (*set_state)(enum n1sdp_c2c_cmd cmd, uint8_t pd_id, uint8_t pd_type);
    /*!
     * \brief API to get a power state in remote chip.
     *
     * \param cmd The C2C command type to issue.
     * \param pd_id The secondary chip's power domain ID.
     * \param state Current power state in power domain pd_id.
     *
     * \retval ::FWK_SUCCESS If operation succeeds.
     * \return One of the possible error return codes.
     */
    int (
        *get_state)(enum n1sdp_c2c_cmd cmd, uint8_t pd_id, unsigned int *state);
    /*!
     * \brief API to issue shutdown/reboot to remote chip.
     *
     * \param cmd The C2C command type to issue.
     * \param type The shutdown/reboot type to issue.
     *
     * \retval ::FWK_SUCCESS If operation succeeds.
     * \return One of the possible error return codes.
     */
    int (*shutdown_reboot)(
        enum n1sdp_c2c_cmd cmd,
        enum mod_pd_system_shutdown type);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_N1SDP_C2C_I2C_H */
