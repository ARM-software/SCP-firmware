\ingroup GroupModules Modules
\defgroup GroupTransport Transport

# Module Transport Architecture

Copyright (c) 2022, Arm Limited. All rights reserved.

# Overview

This module implements a Hardware Abstraction Layer (HAL) API for sending and
receiving inter-processor messages.

Note: The SMT module will be replaced by the Transport module in future.

# Module Design

The transport module provides an interface for modules to send and receive
messages(both SCMI and firmware) via in-band and out-band communication.

The transport module defines a driver interface to transfer/receive data from
drivers like MHU, MHUv2, etc. It also defines a signal interface which is used
to notify the client module on receiving a message.

```
                   +--------+
                   | Shared |
                   | Memory |
                   +--------+
                        ^
+--------+              |                +--------+       +-----------------+
| DRIVER |<-+           |            +-->|  SCMI  |<----->| PROTOCOL MODULE |
+--------+  |           |            |   +--------+       +-----------------+
            |           |            |
+--------+  |    +------+------+     |
| DRIVER |<-+--->|  TRANSPORT  |<----+
+--------+  |    +-------------+     |
            |                        |   +--------+
+--------+  |                        +-->| MODULE |
| DRIVER |<-+                            +--------+
+--------+
```

# Flow

## IN-BAND message communication

### Sending a message
Messages can be sent using in-band communication - where the driver transmits
the message using the channels(for example, the MHUv2 driver sends the message
using the message status registers in the sender channel) by calling the
`send_message()` function and triggering the interrupt(doorbell in case of MHUs)
on the receiver side by calling the `trigger_event()` function in the driver API.

```
+--------+              +---------+                +--------+
| MODULE |              |TRANSPORT|                | DRIVER |
+---+----+              +----+----+                +----+---+
    |                        |                          |
    |                        |                          |
    |      transmit()        |                          |
    +----------------------->|       send_message()     |
    |                        +------------------------->+------+
    |                        |                          |      |
    |                        |                          |      | Copy message to
    |                        |                          |      | message status
    |                        |                          |      | registers
    |                        |                          |      |
    |                        |      return status       |<-----+
    |                        |<-------------------------+
    |                        |                          |
    |                        |                          |
    |                        |      trigger_event()     |
    |                        +------------------------->+------+
    |                        |                          |      |
    |                        |                          |      | Raise interrupt
    |                        |                          |      | on the receiver
    |                        |       return status      |<-----+
    |     return status      |<-------------------------+
    |<-----------------------+                          |
    |                        |                          |
    |                        |                          |
    |                        |                          |
    |                        |                          |

```
###  Receiving a message

When a message is received the transport module is signalled by the driver
module. The transport module then retrieves the message by calling the
`get_message()` function in the driver API. The message is stored in a local
read buffer and the length of the message header and payload is verified and
then the recipient module is signalled by using `signal_message()` function
from the signal API. The recipient module then retrieves the message header
and payload from the transport module and then processes the message.

```
        +--------+                    +-----------+                           +--------+
        | DRIVER |                    | TRANSPORT |                           | MODULE |
        +---+----+                    +-----+-----+                           +----+---+
 Interrupt  |                               |                                      |
----------->+---+                           |                                      |
            |   | isr()                     |                                      |
            |<--+                           |                                      |
            |        signal_message()       |                                      |
            +------------------------------>|                                      |
            |                               |                                      |
            |          get_message()        |                                      |
            |<------------------------------+                                      |
            |                               |                                      |
            +----+Copy message from         |                                      |
            |    |message status registers  |                                      |
            |<---+                          |                                      |
            |       return status           |                                      |
            +------------------------------>|                                      |
            |                               +-----+                                |
            |                               |     |                                |
            |                               |     | Verify the message             |
            |                               |     |                                |
            |                               |     |                                |
            |                               |<----+                                |
            |                               |         signal_message()             |
            |                               +------------------------------------->|
            |                               |          return status               |
            |        return status          |<-------------------------------------+
            |<------------------------------+                                      |
            |                               |                                      |
            |                               |          get_message_header()        |
            |                               |<-------------------------------------+
            |                               |                                      |
            |                               |          get_payload()               |
            |                               |<-------------------------------------+
            |                               |                                      +-----+
            |                               |                                      |     |
            |                               | transport_release_channel_lock() or  |     | Process the message
            |                               |  respond()                           |<----+
            |                               |<-------------------------------------+
            |                               |                                      |
            |                               |           return status              |
            |                               +------------------------------------->|
            |                               |                                      |
```
## OUT-BAND message communication

### Sending a message

Out-band messages are transmitted by the transport module by copying the message
to a shared memory location and then triggering the interrupt on the receiver by
calling the `trigger_event()` in the driver API.

```
+--------+              +---------+                +--------+
| MODULE |              |TRANSPORT|                | DRIVER |
+---+----+              +----+----+                +----+---+
    |                        |                          |
    |                        |                          |
    |      transmit()        |                          |
    +----------------------->+-----+                    |
    |                        |     |                    |
    |                        |     |                    |
    |                        |     | Copy message to    |
    |                        |     | Shared memory      |
    |                        |     |                    |
    |                        |     |                    |
    |                        |<----+                    |
    |                        |      trigger_event()     |
    |                        +------------------------->+----+
    |                        |                          |    |
    |                        |                          |    |raise interrupt on
    |                        |                          |    |the receiver
    |                        |                          |    |
    |                        |     return status        |<---+
    |     return status      |<-------------------------+
    |<-----------------------+                          |
    |                        |                          |
    |                        |                          |
    |                        |                          |
    |                        |                          |
    |                        |                          |
    |                        |                          |
```

###  Receiving a message

When a message is received the transport module is signalled by the driver
module. The transport module then retrieves the message by reading the shared
memory. The message is stored in a local read buffer and the length of the
message header and the payload is verified and then the recipient module is
signalled by using `signal_message()` function from the signal API. The
recipient module then retrieves the message header and payload from the
transport module and then processes the message.

```
        +--------+                      +---------+                      +--------+
        | DRIVER |                      |TRANSPORT|                      | MODULE |
        +---+----+                      +----+----+                      +----+---+
            |                                |                                |
  Interrupt |                                |                                |
----------->+---+                            |                                |
            |   | isr()                      |                                |
            |   |                            |                                |
            |<--+                            |                                |
            |          signal_message()      |                                |
            +------------------------------->+----+                           |
            |                                |    |Check mailbox status       |
            |                                |    |Copy the message header    |
            |                                |    |Verify & copy the payload  |
            |                                |<---+                           |
            |                                |      signal_message()          |
            |                                +------------------------------->|
            |                                |        return status           |
            |         return status          |<-------------------------------+
            |<-------------------------------+                                |
            |                                |     get_message_header()       |
            |                                |<-------------------------------+
            |                                |        get_payload()           |
            |                                |<-------------------------------+
            |                                |                                +------+
            |                                |                                |      |
            |                                |                                |      | Process the message
            |                                |transport_release_channel_lock()|      |
            |                                |     or transport_respond()     |<-----+
            |                                |<-------------------------------+
            |                                |        return status           |
            |                                +------------------------------->|
            |                                |                                |

```

# Use

In order to send/receive in-band messages, the following dependencies are
required:
* In-band message support enabled in the firmware.

## Firmware messages

In order to send firmware message, a module needs to bind to the
`mod_transport_firmware_api` and call `transmit()` function.

## SCMI messages

Transmission and reception of SCMI messages is handled by the scmi module which
in-turn uses the transport module. So, if a module needs to send an scmi message,
it needs to implement the interfaces defined by the scmi module and call the
`scmi_send_message()` function.

## Message reception

The transport module defines the signal interface which must be implemented by
the module so that it can be notified on receiving a message.

# Configuration Example

The following example configures the transport module to be used by the scmi
module to send and receive scmi messages via in-band communication and another
module to send and receive firmware messages via out-band communication using
shared memory.

```C

static const struct fwk_element transport_element_table[] = {
    [0] = {
        .name = "SCP2MCP_SCMI_TRANSPORT",
        .data = &((
            struct mod_transport_channel_config) {
            .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND,
            .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_REQUESTER,
            .in_band_mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
            .driver_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU2,
                    SCP_PLATFORM_MHU_DEVICE_IDX_SCP2MCP,
                    0),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_MHU2,
                    1),
        }),
    },
    [1] = {
        .name = "SCP2MCP_FIRMWARE_TRANSPORT",
        .data = &((
            struct mod_transport_channel_config) {
            .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND,
            .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .policies =  MOD_TRANSPORT_POLICY_INIT_MAILBOX,
            .out_band_mailbox_address = (uintptr_t) SCP_PAYLOAD_NS_A2P_BASE,
            .out_band_mailbox_size = SCP_PAYLOAD_SIZE,
            .signal_api_id = FWK_ID_API_INIT(
                                FWK_MODULE_IDX_TEST,
                                0),
            .driver_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU2,
                    SCP_PLATFORM_MHU_DEVICE_IDX_SCP2MCP,
                    1),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_MHU2,
                    1),
        }),
    },
    [2] = {
        .name = "SCP2MCP_EVENT",
        .data = &((
            struct mod_transport_channel_config) {
            .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE,
            .signal_api_id = FWK_ID_API_INIT(
                                FWK_MODULE_IDX_TEST,
                                0),
            .driver_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU2,
                    SCP_PLATFORM_MHU_DEVICE_IDX_SCP2MCP,
                    2),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_MHU2,
                    1),
        }),
    },
    [3] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    unsigned int idx = 1;
    struct mod_transport_channel_config *config;

    config = (struct mod_transport_channel_config *)(transport_element_table[idx].data);
    config->pd_source_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        platform_get_core_count() + platform_get_cluster_count() +
            PD_STATIC_DEV_IDX_SYSTOP);

    return transport_element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};

```
