/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI performance domain management protocol support.
 */

#ifndef MOD_SCMI_PERF_H
#define MOD_SCMI_PERF_H

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_PERF SCMI Performance Domain Management Protocol
 * \{
 */

/*!
 * \brief Fast Channel Initialisation
 */
#define FCH_INIT(FCH_NUM) \
    { \
        .transport_id = \
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TRANSPORT, FCH_NUM), \
        .transport_api_id = FWK_ID_API_INIT( \
            FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FAST_CHANNELS), \
    }

/*!
 * \brief Agent permissions.
 */
enum mod_scmi_perf_permissions {
    /*! No permissions */
    MOD_SCMI_PERF_PERMS_NONE = 0,

    /*! Permission to set performance level */
    MOD_SCMI_PERF_PERMS_SET_LEVEL = (1U << 0),

    /*! Permission to set performance limits */
    MOD_SCMI_PERF_PERMS_SET_LIMITS = (1U << 1),
};

/*!
 * \brief Fast channels address index
 */
enum mod_scmi_perf_fast_channels_index {
    MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET = 0,
    MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET = 1,
    MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET = 2,
    MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET = 3,
    MOD_SCMI_PERF_FAST_CHANNEL_COUNT = 4,
};
/*!
 *\brief Per-Domain Fast Channel Limit in shared memory.
 */
struct mod_scmi_perf_fast_channel_limit {
    /*! Performance limit max. */
    uint32_t range_max;
    /*! Performance limit min. */
    uint32_t range_min;
};

/*!
 *\brief Fast channels memory offset
 */
enum mod_scmi_perf_fast_channel_memory_offset {
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET = 0,
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET = sizeof(uint32_t),
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET =
        sizeof(uint32_t) + sizeof(struct mod_scmi_perf_fast_channel_limit),
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET =
        sizeof(uint32_t) * 2 + sizeof(struct mod_scmi_perf_fast_channel_limit),
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL = sizeof(uint32_t) * 2 +
        sizeof(struct mod_scmi_perf_fast_channel_limit) * 2
};

/*!
 * \brief Performance limits.
 */
struct mod_scmi_perf_level_limits {
    uint32_t minimum; /*!< Minimum permitted level */
    uint32_t maximum; /*!< Maximum permitted level */
};

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
/*!
 * \brief Fast channel configuration
 */
struct scmi_perf_fch_config {
    fwk_id_t transport_id;
    fwk_id_t transport_api_id;
};

#endif
/*!
 * \brief Performance domain configuration data.
 */
struct mod_scmi_perf_domain_config {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    /* Pointer to the fast channel configuration */
    const struct scmi_perf_fch_config *fch_config;

    /*! Flag indicates whether a particular domain supports fast channel */
    bool supports_fast_channels;
#else
    /*!
     * \brief Domain fast channels.
     *
     * \details Platform Domain fast channel address
     *
     * \note May be set to NULL, in which case support for fast
     *       channels is disabled for the platform.
     */
    uint64_t *fast_channels_addr_scp;

    /*!
     * \brief Agent Domain fast channel address
     *
     * \details Address of shared memory for the agent
     */
    uint64_t *fast_channels_addr_ap;
#endif

    /*! Flag indicating that statistics are collected for this domain */
    bool stats_collected;

    /*!
     * \brief Performance group.
     *
     * \details When there's a mapping of n logical domains from SCMI to one
     *      physical DVFS domain, it is required to indicate the physical domain
     *      associated with each logical domain. See architecture documentation
     *      for further details.
     *
     * \note When not provided, the logical and physical domains are the same.
     *      It is also expected that either none of the domains or all of them
     *      are present.
     */
    fwk_optional_id_t phy_group_id;
};

/*!
 * \brief Performance Plugins domain type
 *
 * \details The plugin's view on performance domains. See architecture
 *      documentation for further details.
 */
enum plugin_domain_type {
    /*!
     * \brief Plugin domain type physical
     *
     * \details A plugin which requires to have a physical domain' data should
     *      choose this option. Note that the plugin will receive `n` updates,
     *      where n is the number of physical domains.
     */
    PERF_PLUGIN_DOM_TYPE_PHYSICAL,

    /*!
     * \brief Plugin domain type logical
     *
     * \details A plugin which requires to have all the logical domains' data
     *      that belong to a physical domain should choose this option. Note
     *      that the plugin will receive `n` updates where `n` is the number of
     *      physical domains.
     */
    PERF_PLUGIN_DOM_TYPE_LOGICAL,

    /*!
     * \brief Plugin domain type full
     *
     * \details A plugin which requires to have all the physical domains' data
     *      in one go should choose this option. Note that the plugin will
     *      receive 1 updates at every iteration. See documentation for further
     *      details.
     */
    PERF_PLUGIN_DOM_TYPE_FULL,

    /*! The number of domain types available for plugins */
    PERF_PLUGIN_DOM_TYPE_COUNT,
};

/*!
 * \brief SCMI Performance Domain Management Protocol configuration data.
 */
struct mod_scmi_perf_config {
    /*! Per-domain configuration data */
    const struct mod_scmi_perf_domain_config (*domains)[];

    /*!
     * \brief Performance domain count
     *
     * \details This is the number of performance domains exposed via SCMI.
     */
    size_t perf_doms_count;

    /*! Fast Channel polling rate */
    uint32_t fast_channels_rate_limit;

    /*! Flag indicating statistics in use */
    bool stats_enabled;

    /*!
     * \brief Whether approximate levels are allowed to be requested.
     *
     * \details When set to true, and no exact match with requested level is
     * found, the next upper level will be used as performance level, as long as
     * it is within current limits.
     */
    bool approximate_level;

    /*! Table of Performance Plugins */
    const struct mod_scmi_plugin_config *plugins;

    /*! Number of Performance Plugins */
    size_t plugins_count;
};

/*!
 * \brief SCMI Performance APIs.
 *
 * \details APIs exported by SCMI Performance Protocol.
 */
enum scmi_perf_api_idx {
    /*! Index for the SCMI protocol API */
    MOD_SCMI_PERF_PROTOCOL_API = 0,

    /*! Index of the updates notification API */
    MOD_SCMI_PERF_DVFS_UPDATE_API = 1,

    /*! Index of the Performance Plugin Handler API */
    MOD_SCMI_PERF_PLUGINS_API = 2,

    /*! Number of APIs */
    MOD_SCMI_PERF_API_COUNT = 3
};

/*!
 * \brief Performance Plugins configuration data.
 */
struct mod_scmi_plugin_config {
    /*! Performance Plugin identifier */
    fwk_id_t id;

    /*! Performance Plugin domain type mode */
    enum plugin_domain_type dom_type;
};

/*!
 * \brief SCMI Perf updates notification API.
 *
 * \details API used by DVFS to notify the Perf layer when either the
 *      limits or level has been changed.
 */
struct mod_scmi_perf_updated_api {
    /*!
     * \brief Inform SCMI Perf that the domain limits have been updated.
     *
     * \param domain_id Domain identifier.
     * \param cookie Context-specific value.
     * \param range_min Min allowed performance level.
     * \param range_max Max allowed performance level.
     */
    void (*notify_limits_updated)(
        fwk_id_t domain_id,
        uintptr_t cookie,
        uint32_t range_min,
        uint32_t range_max);

    /*!
     * \brief Inform SCMI Perf that the domain level has been updated.
     *
     * \param domain_id Domain identifier.
     * \param cookie Context-specific value.
     * \param level The new performance level of the domain.
     */
    void (*notify_level_updated)(
        fwk_id_t domain_id,
        uintptr_t cookie,
        uint32_t level);
};

/*!
 * \defgroup GroupScmiPerformancePolicyHandlers Policy Handlers
 *
 * \brief SCMI Performance Policy Handlers.
 *
 * \details The SCMI policy handlers are weak definitions to allow a platform
 *      to implement a policy appropriate to that platform. The SCMI
 *      performance policy functions may be overridden in the
 *      `product/<platform>/src` directory.
 *
 * \note The `level`/`range` values may be changed by the policy handlers.
 * \note See `product/juno/src/juno_scmi_clock.c` for an example policy
 *      handler.
 *
 * \{
 */

/*!
 * \brief Policy handler policies.
 *
 * \details These values are returned to the message handler by the policy
 *      handlers to determine whether the message handler should continue
 *      processing the message, or whether the request has been rejected.
 */
enum mod_scmi_perf_policy_status {
    /*! Do not execute the message handler */
    MOD_SCMI_PERF_SKIP_MESSAGE_HANDLER = 0,

    /*! Execute the message handler */
    MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER = 1,
};

/*!
 *
 * \brief SCMI Performance Set Level command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Performance Set Level command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] level Level requested to be set.
 * \param[in] agent_id Identifier of the agent requesting the service.
 * \param[in] domain_id Identifier of the performance domain.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int scmi_perf_level_set_policy(
    enum mod_scmi_perf_policy_status *policy_status,
    uint32_t *level,
    unsigned int agent_id,
    fwk_id_t domain_id);

/*!
 * \brief SCMI Performance Set Limits command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Performance Set Limits command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] range_min Minimum level range.
 * \param[in, out] range_max Maximum level range.
 * \param[in] agent_id Identifier of the agent requesting the service.
 * \param[in] domain_id Identifier of the performance domain.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int scmi_perf_limits_set_policy(
    enum mod_scmi_perf_policy_status *policy_status,
    uint32_t *range_min,
    uint32_t *range_max,
    unsigned int agent_id,
    fwk_id_t domain_id);

/*!
 * \}
 */

/*!
 * \brief Performance Plugins performance data.
 *
 * \details This dataset is shared with the plugins. It contains the data
 *      collected from fast-channels and provides a way for the plugins to
 *      request an adjusted limit for the performance driver.
 *      Depending ond the plugin's domain view (physical or logical), levels and
 *      limits can be scalars or arrays.
 */
struct perf_plugins_perf_update {
    /*! Performance domain identifier */
    fwk_id_t domain_id;

    /*! Performance level(s) - This is an input for the plugin */
    uint32_t *level;

    /*! Performance max limit(s) - This is an input for the plugin */
    uint32_t *max_limit;

    /*! Performance min limit(s) - This is an input for the plugin */
    uint32_t *min_limit;

    /*!
     * \brief Adjusted performance max limit(s).
     *
     * \details This is an input/output for the plugin.
     *      This table contains the aggregated adjusted values set out by
     *      previous plugins. Plugins can use this info for their algorithms.
     *      A performance plugin should overwrite these values if wishes to
     *      affect the performance limit for targeted domains.
     */
    uint32_t *adj_max_limit;

    /*!
     * \brief Adjusted performance min limit(s).
     *
     * \details This is an input/output for the plugin.
     *      This table contains the aggregated adjusted values set out by
     *      previous plugins. Plugins can use this info for their algorithms.
     *      A performance plugin should overwrite these values if wishes to
     *      affect the performance limit for targeted domains.
     */
    uint32_t *adj_min_limit;
};

/*!
 * \brief Performance Report data.
 *
 * \details This dataset is shared with the plugins when the performance driver
 *      reports the final applied performance level/limit change.
 */
struct perf_plugins_perf_report {
    /*! Physical (or dependency) domain identifier */
    const fwk_id_t dep_dom_id;

    /*! Performance level */
    const uint32_t level;

    /*! Performance max limit */
    const uint32_t max_limit;

    /*! Performance min limit */
    const uint32_t min_limit;
};

/*!
 * \brief Performance Plugin APIs.
 *
 * \details APIs exported by the Performance Plugins.
 */
enum scmi_perf_plugin_api_idx {
    /*! Index for SCMI Performance Plugin API */
    MOD_SCMI_PERF_PLUGIN_API = 0,

    /*! Number of SCMI Performance Plugins APIs */

    /*!
     * \brief Number of SCMI Performance Plugins APIs
     *
     * \details Other APIs implemented by a performance plugin will start from
     *      this index afterwards.
     */
    MOD_SCMI_PERF_PLUGIN_API_COUNT = 1,
};

/*!
 * \brief Performance Plugins interface.
 *
 * \details The interface that performance plugins should implement to receive
 *      updates on performance.
 */
struct perf_plugins_api {
    /*!
     * \brief Update performance data.
     *
     * \details This function is called periodically to inform the plugin with
     *      the latest performance requests coming from SCMI.
     *
     * \details This callback serves the following purposes:
     *      - periodic tick: the plugins is called at determined time intervals
     *      that can be specified in the configuration and corresponds to the
     *      periodicity of the FastChannels sampling.
     *      - last performance level: the plugin has visibility of the last
     *      updated performance level that is taken from the FastChannels.
     *      - last performance limits: the plugin has visibility of the last
     *      updated performance limits that are taken from the FastChannels.
     *      - adjusted performance limits: the plugin has the opportunity to
     *      affect the final performance limits by writing a performance limit
     *      back to the SCMI Performance.
     *      All the above are put together along with an identifier of the
     *      performance domain being controlled.
     *
     *      A typical scenario for a plugin would be that upon every callback,
     *      the plugin runs its own algorithm which can take advantage of the
     *      periodicity. In this algorithm the plugin can - if required - read
     *      the current performance level and limits for the given domain and,
     *      when necessary, can apply (write) new limits in the `adjusted`
     *      fields. This is expected to be done for each of the relevant
     *      performance domains that the plugin is concerned, unless its view
     *      is _FULL. In the latter case, the callback will execute once only in
     *      the tick period and the plugin will be provided with the entire blob
     *      of performance level/limits for all the domains.
     *      This is made available for plugins whose algorithm needs to take
     *      into account all the performance domains collectively in order to
     *      make a sane decision of the limits to be applied.
     *
     * \param data Performance data
     * \retval ::FWK_SUCCESS or one of FWK_E_* error codes.
     *
     * \return Status code representing the result of the operation.
     */
    int (*update)(struct perf_plugins_perf_update *data);

    /*!
     * \brief Report performance data.
     *
     * \details This function is called once a final performance level limit has
     *      been applied.
     *
     * \param data Performance data
     * \retval ::FWK_SUCCESS or one of FWK_E_* error codes.
     *
     * \note This function is optional. Plugins that are not interested to
     *      receive a report of the last performance transition can omit this
     *      function.
     *
     * \return Status code representing the result of the operation.
     */
    int (*report)(struct perf_plugins_perf_report *data);
};

/*!
 * \brief Performance Request Limits data.
 *
 * \details Data container for the plugin hanlder, contains the requested
 *      limits.
 */
struct plugin_limits_req {
    /*! Performance domain identifier */
    const fwk_id_t domain_id;

    /*! Performance max limit requested */
    const uint32_t max_limit;

    /*! Performance min limit requested */
    const uint32_t min_limit;
};

/*!
 * \brief Plugin handler API - reserved for plugins only
 *
 * \details This interface can be used by any of the performance plugins to
 *      interact with the plugin handler to asyncronously request a performance
 *      level/limit.
 *
 * \warning This API should be used only in circumstances where waiting for the
 *      next update call would cause a significant loss of performance.
 *      Plugins should always try to use the update callback (see above)
 *      whenever possible.
 */
struct perf_plugins_handler_api {
    /*!
     * \brief Set limits
     *
     * \param data Limits request data.
     *
     * \retval ::FWK_E_PARAM Invalid parameter.
     * \retval ::FWK_E_DEVICE The request to the performance driver failed.
     * \retval ::FWK_SUCCESS The operation succeeded. Note that the operation
     *      may complete asyncronously depending on the performance driver.
     *
     * \return Status code representing the result of the operation.
     */
    int (*plugin_set_limits)(struct plugin_limits_req *data);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_PERF_H */
