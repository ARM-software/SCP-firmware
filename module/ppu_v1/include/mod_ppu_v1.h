/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     PPU v1 Driver
 */

#ifndef MOD_PPU_V1_H
#define MOD_PPU_V1_H

#include <mod_power_domain.h>

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModulePPUv1 PPUv1 Driver
 * @{
 */

/*!
 * \brief Indexes of the interfaces exposed by the module.
 */
enum mod_ppu_v1_api_idx {
    /*! Power domain driver API */
    MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER,
    /*! interrupt Service Routine driver API */
    MOD_PPU_V1_API_IDX_ISR,
    /*! System boot API */
    MOD_PPU_V1_API_IDX_BOOT,
    /*! Number of exposed interfaces */
    MOD_PPU_V1_API_IDX_COUNT,
};

/*!
 * \brief Power domain PPU descriptor.
 */
struct mod_ppu_v1 {
    /*! Base address of the PPU registers */
    uintptr_t reg_base;

    /*! PPU's IRQ number */
    unsigned int irq;
};

/*!
 * \brief PPU_V1 module configuration
 */
struct mod_ppu_v1_config {
    /*! Identifier of the power domain notification to register elements for */
    const fwk_id_t pd_notification_id;

    /*!
     * Identifier of the source module or element that is expected to send power
     * domain notifications.
     */
    fwk_id_t pd_source_id;
};

/*!
 * \brief Configuration data of a power domain of the PPU_V1 driver module.
 */
struct mod_ppu_v1_pd_config {
    /*! Power domain type */
    enum mod_pd_type pd_type;

    /*! PPU descriptor */
    struct mod_ppu_v1 ppu;

    /*!
     *  In the case of a core power domain, identifier of the cluster power
     *  domain it belongs to. If the power domain is not a core power domain,
     *  the value of this field is undefined.
     */
    fwk_id_t cluster_id;

    /*!
     * Flag indicating if this domain should be powered on during element
     * init. This flag is only supported for device and system PPUs and should
     * not be set for any other type.
     */
    bool default_power_on;

    /*!
     * \brief Identifier of an entity wishing to be notified when the PPU
     *     transitions out of the OFF state.
     *
     * \note This field may be set to \ref FWK_ID_NONE, in which case no
     *     observer will be set.
     */
    fwk_id_t observer_id;

    /*!
     * \brief Identifier of the power state observer API implemented by
     *     \ref observer_id.
     */
    fwk_id_t observer_api;

    /*!
     * \brief Parameter passed to
     *     \ref mod_ppu_v1_power_state_observer_api::post_ppu_on().
     */
    void *post_ppu_on_param;
};

/*!
 * \brief PPU_V1 Power State Observer API.
 *
 * \details This API should be implemented by any modules that should be
 *     notified when a PPU changes state.
 */
struct mod_ppu_v1_power_state_observer_api {
    /*!
     * \brief Called after a PPU has turned on.
     *
     * \param param Generic configurable parameter.
     */
    void (*post_ppu_on)(void *param);
};

/*!
 * \brief PPU_V1 module ISR API
 */
struct ppu_v1_isr_api {
    /*!
     * \brief Handle a power domain PPU interrupt
     *
     * \param pd_id Identifier of the power domain
     */
    void (*ppu_interrupt_handler)(fwk_id_t pd_id);
};

/*!
 * \brief PPU_V1 module boot API
 */
struct ppu_v1_boot_api {
    /*!
     * \brief Power on a specified power domain
     *
     * \param pd_id Identifier of the power domain
     *
     * \retval FWK_SUCCESS Operation successful.
     * \return One of the standard framework error codes.
     */
    int (*power_mode_on)(fwk_id_t pd_id);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_PPU_V1_H */
