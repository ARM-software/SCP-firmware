SCP-firmware Change Log
=======================

Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.

SCP-firmware - version 2.5.0
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

- The build system does not configure the compiler to avoid code generation of
  unaligned accesses. As ARMv7-M targets are configured to trap on unaligned
  word or halfword accesses, this can result in hardware exceptions.
  [This patch](https://github.com/ARM-software/SCP-firmware/commit/d2a77e0d4d0d369f7504c032a380578a4d145438)
  fixes this issue.


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
