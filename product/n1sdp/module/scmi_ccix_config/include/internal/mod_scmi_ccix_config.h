/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef INTERNAL_SCMI_CCIX_CONFIG_H
#define INTERNAL_SCMI_CCIX_CONFIG_H

#include <stdint.h>

/*
 * SCMI CCIX config ID definition.
 */
#define SCMI_PROTOCOL_ID_CCIX_CONFIG      UINT32_C(0x91)

/*
 * SCMI CCIX config protocol version definition.
 */
#define SCMI_PROTOCOL_VERSION_CCIX_CONFIG UINT32_C(0x10000)

/*
 * SCMI CCIX config Protocol commands ids
 */
enum scmi_ccix_config_command_id {
    /* Command ID for setting ccix config */
    SCMI_CCIX_CONFIG_SET                      = 0x3,
    /* Command ID for getting ccix config*/
    SCMI_CCIX_CONFIG_GET                      = 0x4,
    /* Command ID to initiate protocol credit exchange */
    SCMI_CCIX_CONFIG_EXCHANGE_PROTOCOL_CREDIT = 0x5,
    /* Command ID to configure for system coherency */
    SCMI_CCIX_CONFIG_ENTER_SYSTEM_COHERENCY   = 0x6
};


/*
 * SCMI CCIX PROTOCOL_ATTRIBUTES
 */
#define ADDRESS_MSB_MASK        UINT64_C(0xFFFFFFFF00000000)
#define ADDRESS_LSB_MASK        UINT64_C(0x00000000FFFFFFFF)


struct __attribute((packed)) scmi_ccix_config_mempools_map {
    uint32_t ha_id;
    uint32_t base_msb;
    uint32_t base_lsb;
    uint32_t size_msb;
    uint32_t size_lsb;
};


/*
 * CCIX CONFIG GET
 *
 * agent_count bit field definition
 *
 * reserved[31-24]
 * ha_count[23-16]
 * sa_count[15-8]
 * ra_count[7-0]
 */
#define MAX_HA_MMAP_ENTRIES      4
#define RA_COUNT_MASK            UINT32_C(0x000000FF)
#define RA_COUNT_BIT_POS         0
#define SA_COUNT_MASK            UINT32_C(0x0000FF00)
#define SA_COUNT_BIT_POS         8
#define HA_COUNT_MASK            UINT32_C(0x00FF0000)
#define HA_COUNT_BIT_POS         16

/*
 * link_properties bit field definition
 *
 * opt_tlp[31]
 * msg_packing[30]
 * max_packet_size[29-27]
 * data_credits[26-18]
 * snoop_credits[17-9]
 * request_credits[8-0]
 */
#define REQUEST_CREDITS_MASK         UINT32_C(0x000001FF)
#define REQUEST_CREDITS_BIT_POS      0
#define SNOOP_CREDITS_MASK           UINT32_C(0x0003FE00)
#define SNOOP_CREDITS_BIT_POS        9
#define DATA_CREDITS_MASK            UINT32_C(0x07FC0000)
#define DATA_CREDITS_BIT_POS         18
#define HOST_MAX_PACKET_SIZE_MASK    UINT32_C(0x38000000)
#define HOST_MAX_PACKET_SIZE_BIT_POS 27
#define HOST_MSG_PACK_MASK           UINT32_C(0x40000000)
#define HOST_MSG_PACK_BIT_POS        30
#define HOST_OPT_TLP_MASK            UINT32_C(0x80000000)
#define HOST_OPT_TLP_BIT_POS         31


struct __attribute((packed)) scmi_ccix_config_protocol_get_p2a {
    int32_t  status;
    uint32_t agent_count;
    uint32_t host_mmap_count;
    struct scmi_ccix_config_mempools_map mem_pools[MAX_HA_MMAP_ENTRIES];
    uint32_t link_properties;
};

/*
 * CCIX CONFIG SET
 *
 * config_property bit field definition
 *
 * reserved[31-29]
 * max_packet_size[28-26]
 * opt_tlp[25]
 * msg_packing[24]
 * link_id[23-16]
 * ep_start_bus_number[15-8]
 * traffic_classs[7-0]
 */
#define TRAFFIC_CLASS_MASK       UINT32_C(0x000000FF)
#define TRAFFIC_CLASS_BIT_POS    0
#define EP_START_BUS_NUM_MASK    UINT32_C(0x0000FF00)
#define EP_START_BUS_NUM_BIT_POS 8
#define LINK_ID_MASK             UINT32_C(0x00FF0000)
#define LINK_ID_BIT_POS          16
#define MSG_PACK_MASK            UINT32_C(0x01000000)
#define MSG_PACK_BIT_POS         24
#define OPT_TLP_MASK             UINT32_C(0x03000000)
#define OPT_TLP_BIT_POS          25
#define MAX_PACKET_SIZE_MASK     UINT32_C(0x1C000000)
#define MAX_PACKET_SIZE_BIT_POS  26

struct __attribute((packed)) scmi_ccix_config_protocol_set_a2p {
    uint32_t agent_count;
    uint32_t config_property;
    uint32_t remote_mmap_count;
    struct scmi_ccix_config_mempools_map mem_pools[MAX_HA_MMAP_ENTRIES];
};

struct __attribute((packed)) scmi_ccix_config_protocol_set_p2a {
    int32_t status;
};

/*
 * CCIX CONFIG EXCHANGE PROTOCOL CREDIT
 */
struct __attribute((packed)) scmi_ccix_config_protocol_credit_a2p {
    uint32_t  link_id;
};

/*
 * CCIX CONFIG ENTER SYSTEM COHERENCY
 */
struct __attribute((packed)) scmi_ccix_config_protocol_sys_coherency_a2p {
    uint32_t  link_id;
};


#endif /* INTERNAL_SCMI_CCIX_CONFIG_H */
