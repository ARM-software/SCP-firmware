/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for accessing MXP register.
 */

#ifndef CMN_CYPRUS_MXP_REG_INTERNAL_H
#define CMN_CYPRUS_MXP_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * CMN-Cyprus device type identifiers.
 */
enum cmn_cyprus_device_type {
    DEVICE_TYPE_RESERVED0 = 0x0,
    DEVICE_TYPE_RN_I = 0x1,
    DEVICE_TYPE_RN_D = 0x2,
    DEVICE_TYPE_RESERVED1 = 0x3,
    DEVICE_TYPE_RN_F_CHIB = 0x4,
    DEVICE_TYPE_RN_F_CHIB_ESAM = 0x5,
    DEVICE_TYPE_RN_F_CHIA = 0x6,
    DEVICE_TYPE_RN_F_CHIA_ESAM = 0x7,
    DEVICE_TYPE_HN_T = 0x8,
    DEVICE_TYPE_HN_I = 0x9,
    DEVICE_TYPE_HN_D = 0xA,
    DEVICE_TYPE_HN_P = 0xB,
    DEVICE_TYPE_SN_F_CHIC = 0xC,
    DEVICE_TYPE_SBSX = 0xD,
    DEVICE_TYPE_HN_F = 0xE,
    DEVICE_TYPE_SN_F_CHIE = 0xF,
    DEVICE_TYPE_SN_F_CHID = 0x10,
    DEVICE_TYPE_CXHA = 0x11,
    DEVICE_TYPE_CXRA = 0x12,
    DEVICE_TYPE_CXRH = 0x13,
    DEVICE_TYPE_RN_F_CHID = 0x14,
    DEVICE_TYPE_RN_F_CHID_ESAM = 0x15,
    DEVICE_TYPE_RN_F_CHIC = 0x16,
    DEVICE_TYPE_RN_F_CHIC_ESAM = 0x17,
    DEVICE_TYPE_RN_F_CHIE = 0x18,
    DEVICE_TYPE_RN_F_CHIE_ESAM = 0x19,
    DEVICE_TYPE_HN_S = 0x1A,
    DEVICE_TYPE_RESERVED3 = 0x1B,
    DEVICE_TYPE_MTSX = 0x1C,
    DEVICE_TYPE_HN_V = 0x1D,
    DEVICE_TYPE_CCG = 0x1E,
    DEVICE_TYPE_RN_F_CHIF = 0x20,
    DEVICE_TYPE_RN_F_CHIF_ESAM = 0x21,
    DEVICE_TYPE_SN_F_CHIF = 0x22,
    DEVICE_TYPE_COUNT,
};

/*
 * Get the child node configuration register pointer.
 *
 * \param mxp Pointer to the cross point register.
 *      \pre The pointer must be valid.
 * \param child_index Child node index.
 *      \pre Child node index must be valid.
 * \param periphbase CMN configuration base.
 *
 * \return Child node configuration register pointer.
 */
struct cmn_cyprus_node_cfg_reg *mxp_get_child_node(
    struct cmn_cyprus_mxp_reg *mxp,
    unsigned int child_index,
    uintptr_t periphbase);

/*
 * Verify if a child node (given a mxp base and child index) is an
 * external node from the CMN-Cyprus instance point of view.
 *
 * \param mxp Pointer to the cross point register.
 *      \pre The node pointer must be valid.
 * \param child_index Child index.
 *      \pre The child index must be valid.
 *
 * \retval true if the node is external.
 * \retval false if the node is internal.
 */
bool mxp_is_child_external(
    struct cmn_cyprus_mxp_reg *mxp,
    unsigned int child_index);

/*
 * Returns the number of device ports in the cross point.
 *
 * \param mxp Pointer to the cross point register.
 *      \pre The node pointer must be valid.
 *
 * \retval device port count.
 */
uint8_t mxp_get_device_port_count(struct cmn_cyprus_mxp_reg *mxp);

/*
 * Returns the device type from the MXP's port connect info register.
 *
 * \param mxp Pointer to the cross point register.
 *      \pre The cross point node pointer must be valid.
 * \param port Port number.
 *      \pre The port number should be either 0, 1, 2 or 3.
 *
 * \retval device type.
 */
enum cmn_cyprus_device_type mxp_get_device_type(
    struct cmn_cyprus_mxp_reg *mxp,
    uint8_t port);

/*
 * Verify if the MXP port has CAL connected to it.
 *
 * \param mxp Pointer to the cross point register.
 *      \pre The cross point node pointer must be valid.
 * \param port Port number.
 *      \pre The port number should be either 0, 1, 2 or 3.
 *
 * \retval true if CAL is connected to \param port.
 * \retval false if CAL is not connected to \param port.
 */
bool mxp_is_cal_connected(struct cmn_cyprus_mxp_reg *mxp, uint8_t port);

/*
 * Enable the device connected to the given MXP port by disabling
 * HN-S Isolation for the device.
 *
 * \param mxp Pointer to the cross point node.
 *      \pre The cross point node pointer must be valid.
 * \param port_num Port number.
 *      \pre The port number should be valid.
 * \param device_num Device number.
 *      \pre The device number should be valid.
 *
 * \return Nothing.
 */
void mxp_enable_device(
    struct cmn_cyprus_mxp_reg *mxp,
    uint8_t port_num,
    uint8_t device_num);

#endif /* CMN_CYPRUS_MXP_REG_INTERNAL_H */
