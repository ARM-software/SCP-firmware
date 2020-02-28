SCP-firmware Change Log
=======================

Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.

SCP-firmware - version 2.6.0
============================

New features
------------

- Support for the following platforms has been added :-
    - Juno Arm Development Platform
    - Neoverse N1 Software Development Platform
    - SGM-776
    - RD-N1-Edge
    - RD-N1-Edge Dual-Chip
    - RD-Daniel Config-M

- New modules:
    - DMC500
    - XRP7724
    - CDCEL937
    - I2C
    - dw_apb_i2c (Synopsis DesignWare I2C controller)
    - mock_sensor
    - Debug Module
    - System Info

Changed
-------

- Build System:
    static analysis: Add suppression list for Cppcheck Static Code Analysis
    tools: Add script to check usage of banned functions
    build: enhance entry guards in internal header files

- Documentation:
    DVFS: Document DVFS architecture
    doc: Fix link location for Cppcheck suppression list
    doc: Correct Cppcheck invocation description
    doc: Add notification information to the documentation
    doc: Clarify contributors' and Maintainers' responsibilities
    doc: Update list of maintainers for SCP-firmware
    doc: Add documentation for deferred responses
    doc: Add glossary to Doxygen configuration input

- Framework:
    fwk/notification: Allow use of provided source_id for notify
    thread: Get head of delayed response list
    module: Add FWK_ID_NONE source_id for notifications
    fwk: Delete fwk_module_check_call implementation
    fwk: Extend delayed response support
    fwk: Split delayed response from fwk_multi_thread
    fwk_interrupt: add missing declaration of exported functions
    fwk_thread: Clarify support for delayed responses
    fwk: Rename fwk_errno.h to fwk_status.h
    fwk_errno: Add FWK_PENDING return value
    fwk_module: Extend fwk_module_get_data for sub-elements
    framework: allow product to define notification count

- Modules:
    DVFS: Add support for pending requests
    clock: Add full support for asynchronous drivers
    scmi_clock: Add support for pending requests
    scmi_sensor: Add support for pending responses
    sensor: Add support for asynchronous drivers
    arch/armv7-m: Split exception table from handlers
    arm7-m: Add support for custom default exception handler

SCP-firmware - version 2.5.0
============================

New features
------------

- Modules:
    - CMN600 support
    - DMC620 support
    - apcontext: New module to zero the AP context area

- Platforms:
    - Support for the SGI-575 platform
    - Support for the Neoverse N1 reference design

Note: Juno, Neoverse N1 SDP, Neoverse E1 reference design and SynQuacer products
are NOT part of this release.

Changed
-------

- Build System:
    - Name firmware bin and elf file by the name of the firmware

- Documentation:
    - Emphasized that the order of the modules in the BS_FIRMWARE_MODULES list
      in the firmware definition makefiles is the order in which the modules
      are initialized, bound and started during the pre-runtime phase.
    - Expand Doxygen use of param [out]
    - Improve coding style and rules

- Framework:
    - Improve error codes and their documentation
    - Add fwk_module_get_sub_element_count API
    - Improve unit testing of fwk_module.c
    - Replace base alignment reference type with `max_align_t`

- Modules:
    - sds: Make initialization deferring optional
    - sds: Send notification once SDS is initialized
    - scmi: Subscribe to SMT ready notifications and send notification when
      service is initialized
    - smt: Add logging on mailbox ownership error
    - smt: Send notification when each SMT mailbox is initialized
    - sid: Move to an always available interface
    - system_power: Introduce platform interrupts and enable a configurable
      number of system PPUs
    - cmn600: Add the ccix programming for cmn600

Resolved issues
---------------

- Build System:
    - Disable generation of unaligned accesses

- Framework:
    - list: Always zero list nodes when removed from a list
    - Zero event list node when duplicating an event

- Modules:
    - cmn600: Fix inappropriately-initialised global state
    - cmn600: Stop checking type of external nodes
    - ppu_v0: Ensure bound_id is set when bound by system_power
    - system_power: Fix extented PPU support

SCP-firmware - version 2.4.0
============================

New features
------------

- Module-based architecture with an event-driven execution model:
    - Firmware code is organized into modules, where a module fulfills a well-
      defined role (driver HAL, driver, protocol or service).
    - A framework drives the initialization, orchestration of, and interactions
      between modules.

- Processor-agnostic module code enables firmware portability across systems:
    - Processor-dependent features (e.g. interrupt handling) are abstracted from
      the modules by the framework.
    - The framework relies on an interface whose implementation is processor-
      dependent to provide these features.

- Module configurability easing the re-use of modules on different platforms:
    - Strict split between the hardware and software feature configuration data
      and module code.

- A native framework test suite provides rapid development and validation of the
  core framework implementation.

- Support for Armv7-M control processor architecture based on CMSIS v5 and Keil
  RTX 5.

- Provides system initialization support to enable boot of the application
  cores.

- Provides the following runtime services:
    - Power domain management
    - System power management
    - Performance domain management (Dynamic voltage and frequency scaling)
    - Clock management
    - Sensor management

- Provides a reference implementation of the System Control and Management
  Interface v1.0 (SCMI, platform-side). The SCMI specification can be found
  [here](http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf).

- Provides a build system supporting:
    - The GNU Arm Embedded and Arm Compiler 6 toolchains
    - Platforms with multiple firmware images

- In-source Doxygen documentation

- Support for the SGM-775 platform

Known issues
------------

- The Juno platform is extremely limited with regards to available memory.
  As a result the debug experience is poor, and the ability to add new
  features severely restricted.
- arm-compiler-6 does not support LTO with our build system. No runs
  can be done when the SCP-firmware is built with arm-compiler-6 due
  to space limitations.
