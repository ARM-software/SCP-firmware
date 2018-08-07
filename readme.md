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

Prerequisites (Tools)
---------------------

To properly build SCP/MCP firmware for a target product, the following tools are
required:

- Git
- [GNU Arm Embedded Toolchain ("6-2017-q2-update" or later)](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
- GNU Make
- Python 3 (3.5.0 or later)

In addition, the following tools are recommended:

- Doxygen (1.8.0 or later): Required to build supporting documentation
- GCC (6.0 or later): Required to build framework tests that run on the build
    host

Prerequisites (CMSIS Libraries)
-------------------------------

SCP-firmware requires the use of components from the Cortex Microcontroller
System Interface Standard (CMSIS) Software Pack, specifically the CMSIS Core
and CMSIS Real-Time Operating System (RTOS) components. The CMSIS Software pack
is included as a Git submodule.

To retrieve the CMSIS Software pack, just initialize and update the submodule
of the repository.

    $> git submodule update --init

The required CMSIS components are now present and the correct version has been
selected.

Documentation
-------------

If Doxygen is available on the system containing SCP-firmware then comprehensive
documentation can be generated. The complete set of documentation is compiled
into bundles in HTML, LaTeX, and XML formats and placed in the *build/doc*
directory. This documentation includes:

- A *Readme* section (the content of this file)
- The BSD-3-Clause license under which this software and supporting files are
    distributed
- An overview of the framework on which SCP-firmware is written,
    including information on how components of the software must interact with
    the framework and with each other.
- An overview of the build system and the project directory structure
- The project's coding style
- Source documentation for the SCP-firmware framework
- Source documentation for modules that are distributed as part of the product

From within the SCP-firmware root directory Doxygen can be invoked using the
top-level Makefile:

    $> make doc

Building Products
-----------------
To build a product the basic command format for invoking 'make' is:

    $> make [TARGET] <PRODUCT=<name>> [OPTIONS]

It is not necessary to provide a target since the default target for the product
will build all the firmware contained within the product.

The 'help' target provides further information on the arguments that can be
given:

    $> make help

The framework includes a suite of tests that validate its core functionality.
These can be run on the build host using:

    $> make test

For more guidance on the build system, refer to the full set of documentation
that includes the 'Build System' chapter.

License
-------

The software is provided under a [BSD-3-Clause license](https://spdx.org/licenses/BSD-3-Clause.html).

Feedback and Support
--------------------

Arm welcomes any feedback on SCP-firmware. Please contact the maintainers (see
maintainers.md).

To request support please contact Arm by email at support@arm.com. Arm licensees
may also contact Arm via their partner managers.
