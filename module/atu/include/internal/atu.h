/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ATU_INTERNAL_H
#define ATU_INTERNAL_H

#include <internal/atu_delegate_utils.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief ATU Register Definitions
 */
// clang-format off
struct atu_reg {
    /*! ATU Build Configuration Register */
    FWK_R uint32_t ATUBC;
    /*! ATU Configuration Register */
    FWK_RW uint32_t ATUC;
    /*! ATU Interrupt Status Register */
    FWK_R uint32_t ATUIS;
    /*! ATU Interrupt Enable Register */
    FWK_RW uint32_t ATUIE;
    /*! ATU Interrupt Clear Register */
    FWK_RW uint32_t ATUIC;
    /*! ATU Mismatched Address Register */
    FWK_R uint32_t ATUMA;
    /*! Reserved */
    uint32_t RESERVED0[2];
    /* ATU Right Shifted Start Logical Address n Register */
    FWK_RW uint32_t ATURSSLA[32];
    /* ATU Right Shifted End Logical Address n Register */
    FWK_RW uint32_t ATURSELA[32];
    /*! ATU Region Add Value Low n Register */
    FWK_RW uint32_t ATURAV_L[32];
    /*! ATU Region Add Value High n Register */
    FWK_RW uint32_t ATURAV_H[32];
    /*! ATU Region Output Bus Attributes n Register */
    FWK_RW uint32_t ATUROBA[32];
    /*! ATU Region General Purpose n Register */
    FWK_RW uint32_t ATURGPV[32];
    /*! Reserved */
    uint32_t RESERVED1[811];
    /*! Peripheral ID 4 */
    FWK_R uint32_t PIDR4;
    /*! Reserved */
    uint32_t RESERVED2[2];
    /*! Peripheral ID 0 */
    FWK_R uint32_t PIDR0;
    /*! Peripheral ID 1 */
    FWK_R uint32_t PIDR1;
    /*! Peripheral ID 2 */
    FWK_R uint32_t PIDR2;
    /*! Peripheral ID 3 */
    FWK_R uint32_t PIDR3;
    /*! Component ID 0 */
    FWK_R uint32_t CIDR0;
    /*! Component ID 1 */
    FWK_R uint32_t CIDR1;
    /*! Component ID 2 */
    FWK_R uint32_t CIDR2;
    /*! Component ID 3 */
    FWK_R uint32_t CIDR3;
};
// clang-format on

/*!
 * \brief ATU device context.
 */
struct atu_device_ctx {
#if defined(BUILD_HAS_ATU_MANAGE)
    /*! ATU register */
    struct atu_reg *atu;

    /*! Number of translation regions supported by the ATU */
    uint8_t max_atu_region_count;

    /*! Page size granularity */
    uint8_t page_size;

    /*! SOC's Physcial address width */
    uint8_t phy_addr_width;

    /*! Total number of active ATU regions */
    uint8_t active_regions_count;

    /*! Table of active ATU translation regions */
    struct atu_region_map *active_regions;

    /*!
     * Table of ATU region identifiers. Each entry in this table denotes the
     * entity that owns the corresponding ATU region entry in the
     * \ref active_regions table.
     */
    fwk_id_t *owner_id_list;
#endif
#if defined(BUILD_HAS_ATU_DELEGATE)
    /*! Flag to indicate if a new message has been received */
    bool is_new_msg_received;

    /*! Transport API to send/respond to a message */
    const struct mod_transport_firmware_api *transport_api;

    /*! Timer API */
    const struct mod_timer_api *timer_api;

    /* Variable to hold the most recently received response message */
    struct atu_msg_buffer recv_msg_buffer;

    /* Payload buffer size */
    size_t payload_buffer_size;
#endif
    /*! ATU device configuration data */
    const struct mod_atu_device_config *config;

    /*! Pointer to framework helper functions */
    const struct mod_atu_ops *atu_ops;

    /* Pointer to the ATU API */
    const struct mod_atu_api *atu_api;
};

/*!
 * \brief ATU module context.
 */
struct mod_atu_ctx {
    /* Table of device context */
    struct atu_device_ctx *device_ctx_table;

    /* Number of devices in the device context table */
    unsigned int device_count;
};

/*!
 * \brief Structure to store the pointers to ATU module framework helper
 * functions
 */
struct mod_atu_ops {
    /*!
     * \brief Pointer to module context init function
     *
     * \param atu_ctx_param Pointer to the module context structure.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered.
     */
    int (*atu_init_shared_ctx)(struct mod_atu_ctx *atu_ctx_param);

    /*!
     * \brief Pointer to module init helper function
     *
     * \param device_ctx Pointer to the device context
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*atu_init)(struct atu_device_ctx *device_ctx);

    /*!
     * \brief Pointer to module element init helper function
     *
     * \param device_id Identifier of the device
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*atu_device_init)(fwk_id_t device_id);

    /*!
     * \brief Pointer to module bind helper function
     *
     * \param device_ctx Pointer to the device context
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*atu_bind)(struct atu_device_ctx *device_ctx);

    /*!
     * \brief Pointer to the module start helper function
     *
     * \param device_ctx Pointer to the device context
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*atu_start)(struct atu_device_ctx *device_ctx);
};

#if defined(BUILD_HAS_ATU_MANAGE)
/*!
 * \brief Get pointer to the ATU module framework helper functions structure.
 *
 * \param[out] atu_ops Double pointer to framework helper functions structure.
 *
 * \return Nothing.
 */
void atu_get_manage_ops(const struct mod_atu_ops **atu_ops);
#endif

#if defined(BUILD_HAS_ATU_DELEGATE)
/*!
 * \brief Get pointer to the ATU module framework helper functions structure.
 *
 * \param[out] atu_ops Double pointer to framework helper functions structure.
 *
 * \return Nothing.
 */
void atu_get_delegate_ops(const struct mod_atu_ops **atu_ops);

/*!
 * \brief Get pointer to the ATU signal API.
 *
 * \param[out] api Double pointer to the signal API.
 *
 * \return Nothing.
 */
void atu_get_signal_api(const void **api);
#endif

#endif /* ATU_INTERNAL_H*/
