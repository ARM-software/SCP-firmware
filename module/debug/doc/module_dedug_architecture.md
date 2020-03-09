\ingroup GroupModules Modules
\defgroup GroupDebug Debug HAL

# DEBUG Module Architecture            {#module_debug_architecture_architecture}

## Overview                                {#module_debug_architecture_overview}

This module implements a Hardware Abstraction Layer (HAL) API for SCMI
transactions targeting the DEBUG power domain.

The DEBUG module provides an interface for modules to get and set the required
status of the entire hardware debug infrastructure that can be available on a
platform when it is required to perform a set of operations other than a
simple power domain state change.

The DEBUG module defines a driver interface on which it relies to get and set
the power status of the debug system. It also provides a mechanism to allow the
driver to submit requests to the HAL to enable/disable the debug system. This
can be useful when an external device connected to the platform, such as a
debugger, requires the debug infrastructure available to be functional.

All the platform-specific operations can be implemented in the driver.

The DEBUG module follows the deferred response architecture to deal with
asynchronous drivers.

### DEBUG software architecture            {#module_debug_software_architecture}

The following diagram shows the software architecture. The left side is
without the debug module whereas right side is with the debug module.

Power domain module is sufficient in simple cases such as simply
enabling/disabling DBGSYS power domain. However for platform specific needs,
debug module can be plugged in.


          standard - w/o debug   |    with debug module and debug driver
                                 |
              +------------+     |           +------------+
              |    SCMI    |     |           |   SCMI     |
              +------------+     |           +------------+
                                 |
                                 |
              +------------+     |           +------------+
              |  SCMI_PD   |     |           |  SCMI_PD   |
              +------------+     |           +------------+
                                 |
                                 |
              +------------+     |           +------------+
              |    PD      |     |           |   DEBUG    |
              +------------+     |           +------------+
                                 |
                                 |                                --+
              +------------+     |           +------------+         |
              |   DRIVER   |     |           |DEBUG-DRIVER|         |  platform
              +------------+     |           +------------+         |  specific
                                 |                                --+
                                 |
                                 |   +-----------+  +-----------+
                                 |   | MODULE A  |  | MODULE B  |
                                 |   +-----------+  +-----------+
                                 |

In the right-hand side case, the debug module offers a separation between the
scmi_power_domain module and other HALs that will be used to fulfill the
platform's needs. The debug driver will then contain all the logic without
affecting core modules.

### DEBUG asynchronous operations                {#module_debug_async_operation}

The debug module follows the deferred response architecture. The driver can be
either asynchronous or synchronous. There is only a minimal difference compared
to the standard architecture. This is basically to keep the implementation of
debug module compact.
Specifically, when the DEBUG module processes the request event, it will not
submit and defer the response event in case the original request came from the
driver.
The driver will then inform the HAL via the drv_resp api, and the HAL
won't take any further action, but updating its internal state.

                        DEBUG                   DEBUG-DRIVER
                          ~                          ~
                          |                          |
                          |                          |
            EV  >>>>>>>>+-+-+                        |
                        |   |     set_enabled        |
                        |   +--------------------->+-+-+
                        |   |                      |   +-----+
                        |   |                      |   |     |
                        |   |          PENDING     |   +<----+ ev
                        |   +<---------------------+-+-+
                        |   |                        |
                        +-+-+                        |
                          |                          |
                          |                          |
                        +-+-+    drv_rsp           +-+-+<------ ISR/ev
                        |   +<---------------------+   |
                        |   |                      |   |
                        |   +--------------------->+   |
                        +-+-+                      |   |
                          |                        +-+-+
                          |                          |
                          ~                          ~


    LEGEND
    EV           : The module processes the request event originated by the
                   driver through the driver_input_api
    ev           : (optional) A driver may internally generate an event to
                   process later the request
    ISR/ev       : Either and ISR or an event will cause the driver to call
                   the driver_response api.
    drv_rsp      : driver_response api.
    PENDING      : The call returns with the FWK_PENDING status code
    set_enabled  : A request to the driver
    ----->       : Direct call/return API
    >>>>>>       : Asynchronous call via the event interface


### DEBUG set/get operations                   {#module_debug_set_get_operation}

The DEBUG module offers two ways to submit requests for enable/disable the
implemented features. The first is through its module API and the second is
via the driver_input api.
The following diagrams shows the latter case where the driver will forward the
request coming from the hardware to the HAL. This mechanism ensures that the
asynchronous support can be preserved.

                        DEBUG                   DEBUG-DRIVER
                          ~                          ~
                          |                          |
                          |                        +-+-+<----- ISR-PWR-UP
                          |      enable_request    |   |
                        +-+-+<---------------------+   |
                        |   |                      |   |
                        |   +-----+                |   |
                        |   |     |                |   |
                        |   +<----+ req_drv        |   |
                        |   |                      |   |
                        +-+-+--------------------->+   |
                          |                        |   |
                          |                        +-+-+
                          |                          |
                          ~                          ~

    LEGEND
    ISR-PWR-UP     : An interrupt is raised from the platform and the driver
                     acts
    enable_request : The driver calls the driver_input api to request the HAL
                     to enable the debug features
    req_drv        : The HAL generates the request_event
    ----->         : Direct call/return API
