/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) v3 Device Driver.
 */

#ifndef MOD_MHU3_H
#define MOD_MHU3_H

#ifdef BUILD_HAS_MOD_TIMER
#    include <fwk_id.h>
#endif

#include <fwk_macros.h>

#include <stdint.h>

#define SCP_MHU3_PBX_MBX_SIZE (64 * 1024)

/* Mask to apply to MBX_FCH_CTRL to enable Fast Channel Mailbox interrupts */
#define SCP_MHU3_MBX_INT_EN (4U)

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
    MOD_MHU3_CHANNEL_TYPE_FCH,
};

/*!
 * \brief API indices
 */
enum mod_mhu3_api_idx {
#ifdef BUILD_HAS_MOD_TRANSPORT
    /*! TRANSPORT driver API */
    MOD_MHU3_API_IDX_TRANSPORT_DRIVER,
#endif
    /*! MHU3 API count */
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

/*!
 * \brief Fast channel direction, Fast Channels are unidirectional
 */
enum mod_fch_direction {
    /*! Direction in: Channel used by this processor to receive messages */
    MOD_MHU3_FCH_DIR_IN,
    /*! Direction out: Channel used by this processor to send messages */
    MOD_MHU3_FCH_DIR_OUT,
    /*! Direction count */
    MOD_MHU3_FCH_DIR_COUNT,
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
    /*!
     * Fast Channel idx:
     * offset of the word in PFCW<n>_PAY
     * e.g for
     *     idx 0 is PFCW_PAY + (0 * 4)
     *     idx 1 is PFCW_PAY + (1 * 4)
     * grup_num marks the end of the group
     * e.g.
     *     for idx 0 & grup_num 2 it
     *     is PCFW_PAY + (0 * 4) to PCFW_PAY + (0 * 4 + 4 * 2)
     */
    uint16_t idx;
    /*! Fast Channel Group number for the given Fast Channel
     * There can be between 1-32 Fast Channel Groups.
     */
    uint8_t grp_num;
    /*! Fast Channel Direction */
    enum mod_fch_direction direction;
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

    /*! Base address of the registers of the incoming MHU, MBX
     * (base address of the paired PBX on the target with current processor)
     */
    uintptr_t in;

    /*! Base address of the registers of the outgoing MHU, PBX
     * (base address of the paired MBX on the target with current processor)
     */
    uintptr_t out;

    /*! Base address of the registers of the incoming MHU, MBX
     * (as seen by the firmware/OS running on the target processor)
     */
    uintptr_t in_target;

    /*! Base address of the registers of the outgoing MHU, PBX
     * (as seen by the firmware/OS running on the target processor)
     */
    uintptr_t out_target;

    /*! Channel configuration array */
    struct mod_mhu3_channel_config *channels;

#ifdef BUILD_HAS_MOD_TIMER
    /*! Timer identifier */
    fwk_id_t timer_id;

    /*! Response wait timeout in micro seconds */
    unsigned int resp_wait_timeout_us;
#endif
};

/*!
 * \brief Build an MHU v3 Doorbell channel configuration
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
 * \brief Build an MHU v3 Fast channel configuration
 *
 * \note This macro expands to a designated fast channel configuration,
 *     and can be used to initialize a ::mod_mhu3_channel_config.
 *
 * \details Example usage:
 *      \code{.c}
 *      struct mod_mhu3_channel_config ch = MOD_MHU3_INIT_FCH(0, 1, 0)
 *      \endcode
 *
 * \param FCH_IDX Channel number.
 * \param FCH_GROUP_NUM Fast Channel Group number for the given Fast Channel.
 * \param FCH_DIRECTION Fast Channel direction in or out.
 *
 * \return Element identifier.
 */
#define MOD_MHU3_INIT_FCH(FCH_IDX, FCH_GROUP_NUM, FCH_DIRECTION) \
    { \
        .type = MOD_MHU3_CHANNEL_TYPE_FCH, \
        { \
            .fch = { \
                .idx = FCH_IDX, \
                .grp_num = FCH_GROUP_NUM, \
                .direction = FCH_DIRECTION, \
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
