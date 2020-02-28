SCP-firmware - version 2.6
==========================

Copyright (c) 2011-2020, Arm Limited. All rights reserved.

References
----------

[1] Power Control System Architecture - DEN0050C (Please contact Arm directly to
obtain a copy of this document)

[2] [System Control and Management Interface - DEN0056A](http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf)

[3] Power Policy Unit - DEN0051C (Please contact Arm directly to obtain a copy
of this document)

[4] [System Guidance](https://developer.arm.com/products/system-design/system-guidance)

Introduction
------------

There is a strong trend in the industry to provide microcontrollers in systems
to abstract various power, or other system management tasks, away from
Application Processors (APs). The Power Control System Architecture (PCSA) [1]
describes how systems can be built following this approach.

The PCSA defines the concept of the System Control Processor (SCP), a dedicated
processor that is used to abstract power and system management tasks away from
application processors.

Similar to the SCP, the Manageability Control Processor (MCP) follows the same
approach with the goal of providing a management entry-point to the System on
Chip (SoC) where manageability is required, such as on a SoC targeting servers.

SCP-firmware provides a software reference implementation for the System Control
Processor (SCP) and Manageability Control Processor (MCP) components found in
several Arm Compute Sub-Systems.

Functionality
-------------

- Initialization of the system to enable application core boot
- Runtime services:
    - Power domain management
    - System power management
    - Performance domain management (Dynamic voltage and frequency scaling)
    - Clock management
    - Sensor management
- System Control and Management Interface (SCMI, platform-side)
- Support for the GNU Arm Embedded and Arm Compiler 6 toolchains
- Support for platforms with several control processors

Platforms
---------

Various builds of the release have been tested on the following ARM Fixed
Virtual Platforms (FVPs):

- System Guidance for Mobile platform SGM-775 (Version 11.3 Build 42)
- System Guidance for Infrastruture SGI-575 (Version 11.3 Build 42)
- Neoverse N1 reference design (Version 11.6 Build 45)
- System Guidance for Mobile platform SGM-776 (Version 11.6 Build 45)
- Juno reference design (Please contact Arm directly to obtain the Juno FVP)
- RD-N1-Edge Dual-Chip reference design  (Please contact Arm directly to obtain the
  RdN1EdgeX2 FVP)
- RD-Daniel Config-M reference design  (Please contact Arm directly to obtain the
  RdDaniel-CfgM FVP)

License
-------

The software is provided under a [BSD-3-Clause license](https://spdx.org/licenses/BSD-3-Clause.html).

Getting started
---------------

See user_guide.md for instructions on how to get, install, build and use
SCP-firmware on supported Arm platforms.

Feedback and Support
--------------------

Arm welcomes any feedback on SCP-firmware. Please contact the maintainers (see
maintainers.md).

To request support please contact Arm by email at support@arm.com. Arm licensees
may also contact Arm via their partner managers.
