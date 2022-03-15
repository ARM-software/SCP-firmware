/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) v3 Device Driver.
 */

#ifndef MOD_MHU3_H
#define MOD_MHU3_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupMHUv3 Message Handling Unit (MHU) v3 Driver
 * \{
 */

/*! \brief MHU v3 Channel Types */
enum mod_mhu3_channel_type {
    /*! Doorbell channel type */
    MOD_MHU3_CHANNEL_TYPE_DBCH,
    /* Fast channel type */
    MOD_MHU3_CHANNEL_TYPE_FCH_SINGLE,
    MOD_MHU3_CHANNEL_TYPE_FCH_GROUP,
};

/*!
 * \brief API indices
 */
enum mod_mhu3_api_idx {
    MOD_MHU3_API_IDX_COUNT,
};

/*! \brief Doorbell channel configuration
 *
 *  \details   In MHU3 we can have up to 128 doorbell channels and each
 *      channel can represent 32 independent events indicated
 *      by a flag within the doorbell register.
 *      Hence in MHU3 a channel is represented by its MBX and PBX
 *      number(s) (e.g SCP -> AP) and the position of the event flags
 */
struct mod_mhu3_dbch_config {
    /*! PBX channel number */
    uint32_t pbx_channel : 8;
    /*!
     * Each doorbell channel has 32 bits which represents
     * 32 independent 'events' per doorbell channel
     * flag_pos indicates position of the bit(flag) within doorbell
     * register on which this channel is expected to send or
     * receive communication.
     */
    uint32_t pbx_flag_pos : 8;

    /*! MBX channel number */
    uint32_t mbx_channel : 8;
    /*! MBX flag position within channel */
    uint32_t mbx_flag_pos : 8;
};

/*! \brief Fast channel configuration
 *
 *  \details   In MHU3 the number of Fast Channels that can be supported is
 *      configurable between:
 *      * 1-1024 when the Fast Channel word-size is 32 bits.
 *      * 1-512 when the Fast Channel word-size is 64 bits.
 *      Due to the possible number of Fast Channels, they are controlled in
 *      groups called Fast Channel Groups. There can be between 1 and 32 Fast
 *      Channel Groups in a Postbox or Mailbox each containing between 1 and 32
 *      Fast Channels.
 */
struct mod_mhu3_fc_config {
    /*! Fast Channel or Fast Channel group identifier */
    uint32_t idx : 8;
    /*! Number of Fast channels in a group */
    uint32_t grp_num_channels : 8;
    /*! Reserved field */
    uint32_t reserved : 16;
};

/*! \brief Configuration of a channel between MHU(S/R) <=> MHU(R/S)
 *
 *  \details Each MHU v3 channel is identified using its type
 *      e.g. Doorbell channel or Fast Channel (see ::mod_mhu3_channel_type)
 *      Each channel includes its associated channel information e.g.
 *      in doorbell channel it will be represented by a PBX channel number, its
 *      flag position within corresponding PBX channel, MBX channel number and
 *      its flag position within corresponding MBX channel.
 */
struct mod_mhu3_channel_config {
    /*! Type of the MHU channel */
    enum mod_mhu3_channel_type type;
    /*! Configuration of the specified channel type */
    union {
        /*! Doorbell channel configuration */
        struct mod_mhu3_dbch_config dbch;
        /*! Fast channel configuration */
        struct mod_mhu3_fc_config fch;
    };
};

/*!
 * \brief MHU v3 device
 *
 * \details Abstract representation of a bidirectional MHU device that consists
 *      of a single receive interrupt line and channel configuration.
 */
struct mod_mhu3_device_config {
    /*! IRQ number of the receive interrupt line */
    unsigned int irq;

    /*! Base address of the registers of the incoming MHU, MBX */
    uintptr_t in;

    /*! Base address of the registers of the outgoing MHU, PBX */
    uintptr_t out;

    /*! Channel configuration array */
    struct mod_mhu3_channel_config *channels;
};

/*!
 * \brief Build an MHU v3 channel configuration
 *
 * \note This macro expands to a designated channel configuration, and can be
 *     used to initialize a ::mod_mhu3_channel_config.
 *
 * \details Example usage:
 *      \code{.c}
 *      struct mod_mhu3_channel_config ch = MOD_MHU3_INIT_DBCH(0, 1, 0, 1)
 *      \endcode
 *
 * \param PBX_CH_NUMBER PostBox channel number.
 * \param PBX_FLAG_POS PostBox flag position.
 * \param MBX_CH_NUMBER Mailbox channel number.
 * \param MBX_FLAG_POS Mailbox flag position.
 *
 * \return Element identifier.
 */
#define MOD_MHU3_INIT_DBCH( \
    PBX_CH_NUMBER, PBX_FLAG_POS, MBX_CH_NUMBER, MBX_FLAG_POS) \
    { \
        .type = MOD_MHU3_CHANNEL_TYPE_DBCH, \
        { \
            .dbch = { \
                .pbx_channel = PBX_CH_NUMBER, \
                .pbx_flag_pos = PBX_FLAG_POS, \
                .mbx_channel = MBX_CH_NUMBER, \
                .mbx_flag_pos = MBX_FLAG_POS, \
            }, \
        } \
    }

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MHU3_H */
