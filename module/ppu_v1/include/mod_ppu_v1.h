/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
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
 * \{
 */

/*!
 * \defgroup GroupModulePPUv1 PPUv1 Driver
 * \{
 */

/*!
 * \brief Default value if the value in the configuration is not set.
 */
#define DEFAULT_NUM_OF_CORES_IN_CLUSTER 8

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
 * \brief PPU_V1 OPERATING MODES
 */
enum ppu_v1_opmode {
    PPU_V1_OPMODE_00,
    PPU_V1_OPMODE_01,
    PPU_V1_OPMODE_02,
    PPU_V1_OPMODE_03,
    PPU_V1_OPMODE_04,
    PPU_V1_OPMODE_05,
    PPU_V1_OPMODE_06,
    PPU_V1_OPMODE_07,
    PPU_V1_OPMODE_08,
    PPU_V1_OPMODE_09,
    PPU_V1_OPMODE_10,
    PPU_V1_OPMODE_11,
    PPU_V1_OPMODE_12,
    PPU_V1_OPMODE_13,
    PPU_V1_OPMODE_14,
    PPU_V1_OPMODE_15,
    /* No valid operating modes after this line */
    PPU_V1_OPMODE_COUNT
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
 * \brief Timer for set_state.
 *
 * \details This structure is required to be filled in PPUv1 config file only
 *          when the timeout feature is required.
 */
struct mod_ppu_v1_timer_config {
    /*!
     * \brief Timer identifier.
     *
     * \details Used for binding with the timer API and waiting for specified
     *          delay after setting the PPU state.
     */
    fwk_id_t timer_id;

    /*!
     * PPU state change wait delay in micro seconds.
     * A valid non-zero value has to be specified when using this feature.
     */
    uint32_t set_state_timeout_us;
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

    /*! Number of cores in this cluster */
    uint32_t num_of_cores_in_cluster;

    /*! Set to true if the PPU is configured to operate in dynamic mode. */
    bool is_cluster_ppu_dynamic_mode_configured;
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
     *  Product specific ppu opmode.
     */
    enum ppu_v1_opmode opmode;

    /*!
     * Flag indicating if this domain should be powered on during element
     * init. This flag is only supported for device and system PPUs and should
     * not be set for any other type. Timeout is not provided at this stage.
     */
    bool default_power_on;

    /*!
     * \brief Identifier of an entity wishing to be notified when the PPU
     *     transitions out of the OFF state.
     *
     * \note This field may be set to ::FWK_ID_NONE, in which case no
     *     observer will be set.
     */
    fwk_id_t observer_id;

    /*!
     * \brief Identifier of the power state observer API implemented by
     *     ::mod_ppu_v1_pd_config::observer_id.
     */
    fwk_id_t observer_api;

    /*!
     * \brief Parameter passed to
     *     ::mod_ppu_v1_power_state_observer_api::post_ppu_on().
     */
    void *post_ppu_on_param;

    /*! Timer descriptor */
    struct mod_ppu_v1_timer_config *timer_config;
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
     * \retval ::FWK_SUCCESS Operation successful.
     * \return One of the standard framework error codes.
     */
    int (*power_mode_on)(fwk_id_t pd_id);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PPU_V1_H */
