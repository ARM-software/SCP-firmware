/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "juno_mhu.h"
#include "juno_scmi.h"
#include "software_mmap.h"

#include <mod_fch_polled.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

static const struct fwk_element element_table[
#ifdef BUILD_HAS_MOD_TRANSPORT_FC
    JUNO_TRANSPORT_CHANNELS_COUNT + 1] = {
    [JUNO_TRANSPORT_SCMI_SERVICE_PSCI] = {
#else
    JUNO_SCMI_SERVICE_IDX_COUNT + 1] = {
    [JUNO_SCMI_SERVICE_IDX_PSCI_A2P] = {
#endif
    /* JUNO_SCMI_SERVICE_IDX_PSCI_A2P */
        .name = "",
        .data =
            &(struct mod_transport_channel_config){
              .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
              .policies =
                  (MOD_TRANSPORT_POLICY_INIT_MAILBOX |
                   MOD_TRANSPORT_POLICY_SECURE),
              .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD_S_A2P_BASE,
              .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
              .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                  FWK_MODULE_IDX_MHU,
                  JUNO_MHU_DEVICE_IDX_S,
                  0),
              .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
              .pd_source_id = FWK_ID_ELEMENT_INIT(
                  FWK_MODULE_IDX_POWER_DOMAIN,
                  POWER_DOMAIN_IDX_SYSTOP),
            }
        },
#ifdef BUILD_HAS_MOD_TRANSPORT_FC
    [JUNO_TRANSPORT_SCMI_SERVICE_OSPM_0] = {
#else
    [JUNO_SCMI_SERVICE_IDX_OSPM_A2P_0] = {
#endif
    /* JUNO_SCMI_SERVICE_IDX_OSPM_A2P_0 */
        .name = "",
        .data =
            &(struct mod_transport_channel_config){
              .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
              .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
              .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD_LOW_A2P_BASE,
              .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
              .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                  FWK_MODULE_IDX_MHU,
                  JUNO_MHU_DEVICE_IDX_NS_L,
                  0),
              .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
              .pd_source_id = FWK_ID_ELEMENT_INIT(
                  FWK_MODULE_IDX_POWER_DOMAIN,
                  POWER_DOMAIN_IDX_SYSTOP),
            }
        },
#ifdef BUILD_HAS_MOD_TRANSPORT_FC
    [JUNO_TRANSPORT_SCMI_SERVICE_OSPM_1] = {
#else
    [JUNO_SCMI_SERVICE_IDX_OSPM_A2P_1] = {
#endif
    /* JUNO_SCMI_SERVICE_IDX_OSPM_A2P_1 */
        .name = "",
        .data =
          &(struct mod_transport_channel_config){
              .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
              .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
              .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD_HIGH_A2P_BASE,
              .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
              .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                  FWK_MODULE_IDX_MHU,
                  JUNO_MHU_DEVICE_IDX_NS_H,
                  0),
              .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
              .pd_source_id = FWK_ID_ELEMENT_INIT(
                  FWK_MODULE_IDX_POWER_DOMAIN,
                  POWER_DOMAIN_IDX_SYSTOP),
            }
        },
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
#    ifdef BUILD_HAS_MOD_TRANSPORT_FC
    [JUNO_TRANSPORT_SCMI_SERVICE_OSPM_P2A] = {
#    else
    [JUNO_SCMI_SERVICE_IDX_OSPM_P2A] = {
#    endif
    /* JUNO_SCMI_SERVICE_IDX_OSPM_P2A */
        .name = "",
        .data =
            &(struct mod_transport_channel_config){
              .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_REQUESTER,
              .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
              .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD_HIGH_P2A_BASE,
              .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
              .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                  FWK_MODULE_IDX_MHU,
                  JUNO_MHU_DEVICE_IDX_NS_H,
                  1),
              .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
              .pd_source_id = FWK_ID_ELEMENT_INIT(
                  FWK_MODULE_IDX_POWER_DOMAIN,
                  POWER_DOMAIN_IDX_SYSTOP),
            }
        },
#endif

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    [JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LEVEL_SET] = {
        .name = "FCH_BIG_LEVEL_SET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_BIG_LEVEL_SET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LIMIT_SET] = {
        .name = "FCH_BIG_LIMIT_SET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_BIG_LIMIT_SET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LEVEL_GET] = {
        .name = "FCH_BIG_LEVEL_GET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_BIG_LEVEL_GET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LIMIT_GET] = {
        .name = "FCH_BIG_LIMIT_GET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_BIG_LIMIT_GET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LEVEL_SET] = {
        .name = "FCH_LITTLE_LEVEL_SET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_LITTLE_LEVEL_SET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LIMIT_SET] = {
        .name = "FCH_LITTLE_LIMIT_SET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_LITTLE_LIMIT_SET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LEVEL_GET] = {
        .name = "FCH_LITTLE_LEVEL_GET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_LITTLE_LEVEL_GET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LIMIT_GET] = {
        .name = "FCH_LITTLE_LIMIT_GET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_LITTLE_LIMIT_GET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LEVEL_SET] = {
        .name = "FCH_GPU_LEVEL_SET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_GPU_LEVEL_SET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LIMIT_SET] = {
        .name = "FCH_GPU_LIMIT_SET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_GPU_LIMIT_SET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LEVEL_GET] = {
        .name = "FCH_GPU_LEVEL_GET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_GPU_LEVEL_GET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LIMIT_GET] = {
        .name = "FCH_GPU_LIMIT_GET",
        .data = &((
            struct mod_transport_channel_config){
            .transport_type =
                MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                JUNO_PLAT_FCH_GPU_LIMIT_GET),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_FCH_POLLED,
                MOD_FCH_POLLED_API_IDX_TRANSPORT),
            }
        ),
    },
    [JUNO_TRANSPORT_CHANNELS_COUNT] = { 0 },
#else
    [JUNO_SCMI_SERVICE_IDX_COUNT] = { 0 },
#endif
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
