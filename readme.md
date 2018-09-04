Readme
======

Copyright (c) 2011-2018, Arm Limited. All rights reserved.

References
----------

[1] Power Control System Architecture - DEN0050B (Please contact Arm directly to
obtain a copy of this document)

[2] [System Control and Management Interface - DEN0056A](http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf)

[3] Power Policy Unit - DEN0051C (Please contact Arm directly to obtain a copy
of this document)

[4] [System Guidance](https://developer.arm.com/products/system-design/system-guidance)

Introduction
------------

There is a strong trend in the industry to provide microcontrollers in systems
to abstract various power, or other system management tasks, away from
application processors (AP). The Power Control System Architecture (PCSA) [1]
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
