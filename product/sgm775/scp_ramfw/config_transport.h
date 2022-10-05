/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_TRANSPORT_H
#define CONFIG_TRANSPORT_H

enum config_transport_channel {
    CONFIG_TRANSPORT_CHANNEL_IDX_A2P,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    CONFIG_TRANSPORT_CHANNEL_IDX_P2A,
#endif
    CONFIG_TRANSPORT_CHANNEL_IDX_COUNT,
};

#endif /* CONFIG_TIMER_H */
