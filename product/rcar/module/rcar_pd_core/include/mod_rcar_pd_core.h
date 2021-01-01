/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_PD_CORE_H
#define MOD_RCAR_PD_CORE_H

#include <config_power_domain.h>

#include <mod_power_domain.h>

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CPU_PWR_OFF         (0x00000003U)
#define MODE_L2_DOWN        (0x00000002U)
#define CORE_PER_CLUSTER_COUNT_MAX 8

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARPdCore PD Core
 * @{
 */

/*!
 * \brief Indexes of the interfaces exposed by the module.
 */
enum mod_rcar_ps_sysc_api_idx {
    /*! Power domain driver API */
    MOD_RCAR_PD_SYSC_API_IDX_POWER_DOMAIN_DRIVER,
    /*! interrupt Service Routine driver API */
    MOD_RCAR_PD_SYSC_API_IDX_ISR,
    /*! System boot API */
    MOD_RCAR_PD_SYSC_API_IDX_BOOT,
    /*! Number of exposed interfaces */
    MOD_RCAR_PD_SYSC_API_IDX_COUNT,
};

/*!
 * \brief Power domain descriptor.
 */
struct mod_rcar_pd_sysc {
    /*! Base address of the registers */
    uintptr_t reg_base;
};

/*!
 * \brief core module configuration
 */
struct mod_rcar_pd_core_config {
    /*! Identifier of the power domain notification to register elements for */
    const fwk_id_t pd_notification_id;

    /*!
     * Identifier of the source module or element that is expected to send power
     * domain notifications.
     */
    fwk_id_t pd_source_id;
};

/*!
 * \brief Configuration data of a power domain of the sysc driver module.
 */
struct mod_rcar_pd_core_pd_config {
    /*! Power domain type */
    enum rcar_pd_type pd_type;

    /*! descriptor */
    struct mod_rcar_pd_sysc ppu;

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
     * \brief Identifier of an entity wishing to be notified when the
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
     * Flag indicating if this domain should be powered on during element init.
     */
    bool always_on;
};

/*!
 * \brief SYSC Power State Observer API.
 *
 * \details This API should be implemented by any modules that should be
 *     notified when a changes state.
 */
struct mod_rcar_pd_sysc_power_state_observer_api {
    /*!
     * \brief Called after a has turned on.
     *
     * \param param Generic configurable parameter.
     */
    void (*post_ppu_on)(void *param);
};

/*!
 * @cond
 */

/* Power domain context */
struct rcar_pd_sysc_pd_ctx {
    /* Power domain configuration data */
    const struct mod_rcar_pd_core_pd_config *config;

    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;

    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;

    /* Context of the parent power domain (used only for core power domains) */
    struct rcar_pd_sysc_pd_ctx *parent_pd_ctx;

    /* Pointer to the power state observer API */
    const struct mod_rcar_pd_sysc_power_state_observer_api *observer_api;

    /* Context data specific to the type of power domain */
    void *data;
    /* Power Domain current state*/
    unsigned int current_state;
};

/* Cluster power domain specific context */
struct rcar_pd_sysc_cluster_pd_ctx {
    /*
     * Table of pointers to the contexts of the cores being part of the
     * cluster.
     */
    struct rcar_pd_sysc_pd_ctx *core_pd_ctx_table[CORE_PER_CLUSTER_COUNT_MAX];

    /* Number of cores */
    unsigned int core_count;
};

/* Module context */
struct rcar_pd_sysc_ctx {
    /* Table of the power domain contexts */
    struct rcar_pd_sysc_pd_ctx *pd_ctx_table;

    /* Number of power domains */
    size_t pd_ctx_table_size;

    /* Log API */
    struct mod_log_api *log_api;
};

/*
 * Internal variables
 */

#define MODE_UNSUPPORTED ~0U

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_PD_CORE_H */
