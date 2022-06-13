\ingroup GroupModules Modules
\defgroup GroupSCMISensorRequester SCMI Sensor Protocol Requester

# SCMI Sensor Requester architecture

Copyright (c) 2022, Arm Limited. All rights reserved.


## Overview

The SCMI Sensor Requester is used to send sensor protocol SCMI commands
to a completer. It also receives the sensor protocol SCMI commands' responses.

The SCMI Sensor Requester implements the driver interface provided by the HAL
sensor. So it is treated as if it was a sensor driver. Hence an entity that
requires to get a sensor value doesn't need to know if the sensor is local or
if is required to access the sensor via the SCMI. This is achievable using the
Sensor HAL.

The SCMI Sensor Requester implements the get_value interface provided by the
HAL. It acts as an asynchronous sensor driver. This is because SCMI Sensor
Requester issues a "Sensor Reading Get" command to the entity responsible of
the actual sensor with no blocking call. When the required entity responds back
the SCMI Sensor Requester handles the response and send the value back to the
Sensor HAL.

## Architecture

The following sequence diagram demonstrates the communication between the
involved modules starting from a client requesting sensor data until it gets
the response.

                            Sequence diagram
    +--------+     +------------+     +-----------------+         +------+
    | Client |     | Sensor HAL |     | SCMI Sensor Req |         | SCMI |
    +--------+     +------------+     +-----------------+         +------+
         | get_data()     |                     |                     |
         |--------------->| get_value()         |                     |
         |                |-------------------->| scmi_send_message() |
         |                |                     |-------------------->|
                                       ..
                                       ..
                                       ..
                                             scmi_sensor_req_message_handler()
         |                | reading_complete()  |<--------------------|
         | put_event()    |<--------------------|                     |
         |<---------------|                     |                     |


## Configuration

The following diagram shows an example configuration for the SCMI sensor
requester in a primary SCP which needs to get a sensor value accessed by
another SCP.

                              Configuration example
    +----------------------------------------------------------------------+
    |  Primary SCP                                                         |
    |----------------------------------------------------------------------|
    |                                                                      |
    | +-------------+ +-------------+ +---------------+   +-------------+  |
    | |Client module| |Sensor HAL   | |SCMI Sensor Req|   | SCMI        |  |
    | |-------------| |-------------| |--------   ----|   |-------------+  |
    | | +---------+ | | +---------+ | | +-----------+ |   | +---------+ |  |
    | | |element_A|-+-+>|element_B|-+-+>|element_C  |-+-+-+>|element_D| |  |
    | | +---------+ | | +---------+ | | +-----------+ | | | +---------+ |  |
    | +-------------+ +-------------+ +---------------+ | +-------------+  |
    |                                                   |                  |
    +---------------------------------------------------+------------------+
                                                        |
                       +----------------------+         |
                       |Secondary SCP         |         |
                       |----------------------|         |
                       | +------------------+ |         |
                       | |Sensor HAL        | |         |
                       | |------------------| |         |
                       | | +--------------+ | |         |
                       | | |element_X     |<+-+---------+
                       | | +--------------+ | | sensor_req_c is configured to
                       +----------------------+ use sensor_hal_x in the
                                                completer by setting its ID.

## Limitations

Currently only the Sensor Reading Get command is implemented.
