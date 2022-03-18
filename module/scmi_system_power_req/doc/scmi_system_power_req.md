\ingroup GroupModules Modules
\defgroup GroupSCMISystemPowerRequester SCMI System Power Requester

# SCMI System Power Requester architecture

Copyright (c) 2022, Arm Limited. All rights reserved.

## Overview

The SCMI System Power Requester module is used to send SCMI System Power
protocol commands, for example setting the state. It will also receive their
responses.

This module is called from the SCMI System Power when it receives an SCMI
command, if the module is implementing the requester. It interacts with this
module in the same way that it already interacts with the Power Domain module.

It implements a set and get state command. The set state is the only one
of the two commands that will construct and forward an SCMI message, as
the system power state is cached for the get command.

## Architecture

The following sequence diagram demonstrates the communication between the
involved modules starting from a client sending SCMI System Power commands,
until it gets the response.

                        Sequence diagram
    +--------+   +--------------+  +-------------+   +------+     +-----------+
    | Client |   | SCMI Sys Pow |  | Sys Pow Req |   | SCMI |     | Transport |
    +--------+   +--------------+  +-------------+   +------+     +-----------+
         | state_set()  |               |                  |                |
         |------------->| state_set()   |                  |                |
         |              |-------------->|                  |                |
         |              | RESP PENDING  |                  |                |
         |              |<--------------|                  |                |
         |              |               | scmi_send()      |                |
         |              |               |----------------->|                |
         |              |               |                  | scmi_send()    |
         |              |               |                  |--------------->|
         |              |               |                  | scmi_response()|
         |              |               |                  |<---------------|
         |              |               | message_handler()|                |
         |              |               |<-----------------|                |


## Configuration

The following diagram shows an example configuration for the SCMI System
Power Requester in an MCP which needs to set a state accessed by another SCP.

The number of protocol_requester_count_max should also be set accordingly
in the SCMI config.

                            Configuration example
    +-----------------------------------------------------------------------+
    |                                MCP                                    |
    |-----------------------------------------------------------------------|
    |                                                                       |
    | +-------------+ +--------------+ +----------------+   +-------------+ |
    | |Client module| |SCMI Sys Power| |SCMI Sys Pow Req|   | SCMI        | |
    | |-------------| |--------------| |----------------|   |-------------+ |
    | | +---------+ | | +----------+ | | +------------+ |   | +---------+ | |
    | | |element_A|-+-+>|element_B |-+-+>| element_C  | |-+-+>|element_D| | |
    | | +---------+ | | +----------+ | | +------------+ | | | +---------+ | |
    | +-------------+ +--------------+ +----------------+ | +-------------+ |
    |                                                     |                 |
    +-----------------------------------------------------+-----------------+
                                                          |
                               +-----------------------+  |
                               |Primary SCP            |  |
                               |-----------------------|  |
                               | +-------------------+ |  |
                               | | SCMI System Power |<+--+
                               | |-------------------| |
                               +-----------------------+


## Limitations

Currently the module only works for a single element.