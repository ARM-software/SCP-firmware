\ingroup GroupArchitecture Architectures
\defgroup GroupOPTEE OP-TEE Architecture

OP-TEE Architecture
===================

# Overview

The OP-TEE architecture provides the basics for interfacing the SCP-firmware
with OP-TEE OS. A dedicated Pseudo Trusted Application (PTA) is created as part
of the OP-TEE os that will be used to run the SCP-firmware.

In addition to the architecture, a number of OP-TEE modules have been added in
modules/optee directory to interface the SCP-Firmware with the ressources
available in the OP-TEE operating system. These modules includes the transport
layer between the SCMI agents and the SCMI server and accesses to various
OP-TEE resources like clocks or reset lines.

# Design

The following diagram shows the SW components invloved in the implementation
of a SCMI server as a OP-TEE Pseudo Trusted Application (PTA). This PTA is
included in the OP-TEE os.

```
                   +----------------------------------------------------------------+
                   | +---------------------+    +--------------------------------+  |
                   | | OPTEE/shared memory |--->| SCMI                           |  |
                   | +---------------------+    +--------------------------------+  |
                   |            ^                      |                  |         |
                   |            |                      v                  v         |
                   | +---------------------+    +-------------+    +--------------+ |
                   | | OPTEE/Mailbox       |    | SCMI clock  |    | SCMI reset   | |
                   | +---------------------+    +-------------+    +--------------+ |
                   |            ^                      |                  |         |
                   |            | SCP-Firmware         v                  v         |
                   |            |               +-------------+    +--------------+ |
                   |            |               | clock       |    | reset domain | |
                   |            |               +-------------+    +--------------+ |
                   |            |                      |                  |         |
                   |            |                      v                  v         |
                   | +---------------------+    +-------------+    +--------------+ |
                   | |  optee architecture |    | OPTEE/clock |    | OPTEE/reset  | |
                   | +---------------------+    +-------------+    +--------------+ |
+---------------+  +------------|----------------------|------------------|---------+
|  Linux kernel |               |                      |                  |
|               |  +------------|----------------------v------------------v---------+
| +-----------+ |  | +---------------------+    +-------------+    +--------------+ |
| | SCMI fwk  | |  | | Pseudo Trusted App  |    | optee clock |    | optee reset  | |
| +-----------+ |  | +---------------------+    +-------------+    +--------------+ |
|       |       |  |            ^                                                   |
|       v       |  |            |                                                   |
| +-----------+ |  |            |                                                   |
| | OPTEE fwk | |  |            |                          OPTEE os                 |
| +-----------+ |  |            |                                                   |
+-------|-------+  +------------|---------------------------------------------------+
        v                       |
      +---------------------------+
      | SMC   EL3                 |
      +---------------------------+

```

# OPTEE modules

## Mailbox

The OPTEE mailbox module implements a doorbell mecnaism. The PTA invoke command
acts like an interrupt to trigger the income of a new request.

## Shared memory

OPTEE supports 2 differents types of shared memory:
- The static shared memory is reserved at boot time. The same memory buffer is
  used to send the request and receive the response. This behavior is similar
  to the module SMT used with the hardware mailbox.
- At the opposite, the Dynamic shared memory mode allocates buffers at runtime.
  One buffer is used for the request and another one for the response. This
  behavior is similar to virtio-scmi.

## Clock

The clock module interfaces the optee-os clock framework with the SCP-firmware clock
module.

## Reset

The reset module interfaces the optee-os reset framework with the SCP-firmware reset
domain module.


