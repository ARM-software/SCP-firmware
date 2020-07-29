/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI base protocol definitions.
 */

#ifndef INTERNAL_SCMI_BASE_H
#define INTERNAL_SCMI_BASE_H

#include <stdint.h>

/*
 * PROTOCOL_ATTRIBUTES
 */
#define SCMI_PROTOCOL_VERSION_BASE UINT32_C(0x10000)

#define SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_PROTOCOLS_POS  0
#define SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_AGENTS_POS     8

#define SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_PROTOCOLS_MASK 0xFF
#define SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_AGENTS_MASK    0xFF00

#define SCMI_BASE_PROTOCOL_ATTRIBUTES(NUM_PROTOCOLS, NUM_AGENTS) \
    ((((NUM_PROTOCOLS) << SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_PROTOCOLS_POS) \
      & SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_PROTOCOLS_MASK) | \
     (((NUM_AGENTS) << SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_AGENTS_POS) \
       & SCMI_BASE_PROTOCOL_ATTRIBUTES_NUM_AGENTS_MASK))

/*
 * BASE_DISCOVER_VENDOR
 */
struct scmi_base_discover_vendor_p2a {
    int32_t status;
    char vendor_identifier[16];
};

/*
 * BASE_DISCOVER_SUB_VENDOR
 */
struct scmi_base_discover_sub_vendor_p2a {
    int32_t status;
    char sub_vendor_identifier[16];
};

/*
 * BASE_DISCOVER_IMPLEMENTATION_VERSION
 * No special structure right now, see protocol_version.
 */

/*
 * BASE_DISCOVER_LIST_PROTOCOLS
 */
struct scmi_base_discover_list_protocols_a2p {
    uint32_t skip;
};

struct scmi_base_discover_list_protocols_p2a {
    int32_t status;
    uint32_t num_protocols;
    uint32_t protocols[];
};

/*
 * BASE_DISCOVER_AGENT
 */
struct scmi_base_discover_agent_a2p {
    uint32_t agent_id;
};

struct scmi_base_discover_agent_p2a {
    int32_t status;
    char name[16];
};

#endif /* INTERNAL_SCMI_BASE_H */
