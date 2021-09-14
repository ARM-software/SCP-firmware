SCP-firmware Change Log
=======================

Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.

SCP-firmware - version 2.9.0
============================

New features
------------

- Framework:
    - Add light events
    - fwk_log: Add option for minimal banner

- New modules:
    - Mock Voltage Domain driver
    - PMI and DWT PMI driver
    - PCIE integration control register
    - CMN Skeena
    - Cadence I2C

- SCMI:
    - scmi_perf: Add performance plugins handler extension

- Build system:
    - CMake: Add support for parallel build
    - CMake: Add support for framework tests

Changed
-------

- Documentation:
    - clock: Add documentation for Clock Management Framework

- Arch:
    - arm: Add interrupt disable/enable inline calls

- Framework:
    - Remove Signals
    - Remove module name parameter

- Modules:
    - clock: Add Clock Management Framework
    - mod_resource_perms: Fix recursion on mod_res_message_id_to_index
    - cmn700: Add support to configure additional non-hash mem regions
    - cmn700: Update root node offset calculation for bigger mesh
    - cmn700: Add hierarchical hashing support with 3 SN mode
    - psu: Add error return from mod_psu_process_bind_request
    - sds: Clear data valid flag after SCP RAM FW has been transferred

- Platforms:
    - juno/scmi_perf: Simplify configuration of FastChannels
    - juno/software_mmap: Remove unused SCMI_FAST_CHANNEL_SIZE
    - juno: Provide PMI and DWT PMI config
    - n1sdp: Enable non-secure CoreSight access in C2C setup
    - n1sdp: Add dynamic calculations of PLL parameters
    - morello: Restructure dmc_bing module
    - rdv1mc, sgi575: Add config for MCP
    - Add support for tc1
    - tc0: Add SCMI FastChannels configuration
    - tc0: Use separate PLLs for seperate core types
    - tc0: Add example configuration for Voltage Domain
    - tc0: Enable DVFS for Matterhorn ELP Arm cores
    - tc0: Update SCP PIK
    - rdv1mc: Add support for DVFS and SCMI Perf
    - rdv1mc: Enable fast channel
    - rdv1mc: Add mock PSU for dvfs
    - rdn2: Update register layout for pik's in rdn2 platform
    - rdn2: Disable NEWLIB_NANO for RD-N2 platform
    - rdn2: Add support for variant 1 of rdn2 platform
    - rdn2: Add ecam and mmio memory regions in cmn700 config
    - rdn2: Update NCI GPV region 0 in cmn700 config
    - rdn2: Add all NCI GPV regions in cmn700 config
    - rdn2: Add config data for pcie integ control module
    - rdn2: Add support for mcp ramfw
    - rdv1: Add config data for mcp romfw bootloader module
    - rdv1: Add config data for clock module in MCP RAM firmware
    - rdv1: Add config data for pl011 UART module in mcp ramfw
    - rdv1: Add configuration data for MPU module in mcp ramfw
    - rdv1: Add generic timer config data for mcp ramfw
    - rdv1: Add timer HAL config data for mcp ramfw
    - rdv1: Add platform specific definitions for rtx
    - rdv1: Add mcp_platform module for rdv1
    - rdv1: Add build support for mcp ram firmware
    - rdv1: Add debugger CLI module config data for mcp ramfw
    - synquacer: Add shutdown handler in ppu_v0_synquacer module

- MISRA C:2012:
    - SCMI, Framework: Fix rule 9.5 violations
    - mod_resource_perms: Fix rule 16.5 violations
    - module: Fix rule 10.3 violations
    - fwk: Fix rules 11.9, 16.3 and 17.7 violations
    - arch: Fix rule 15.6 violations
    - arch/armv7-m: Fix rules 10.1 and 17.7 violations

SCP-firmware - version 2.8.0
============================

New features
------------

- Support for the following platforms has been added:
    - Morello platform
    - RD-N2 platform (initial support)

- New modules:
    - apremap (AP remap)
    - CMN-700 interconnect
    - mock_clock

- Framework:
    - fwk: Add fwk_thread_signal API
    - fwk: Add MT fwk_thread_put_signal API
    - framework: add optional identifier support

- Renamings:
    - RD-Daniel is now RD-V1
    - RD-Daniel Config-XLR is now RD-V1-MC
    - module: rename cmn_rhodes to cmn-650

- MISRA C:2012
    - Mandatory fixes have been resolved

- SCMI:
    - Sensor protocol: Add support for SENSOR_TRIP_POINT_CONFIG
    - Sensor protocol: Add support for SENSOR_TRIP_POINT_NOTIFY

- Build system:
    - Support for CMake has been added.

- Doc:
    - Add cmake_readme.md

Changed
-------

- Documentation:
    - doc: SCP Firmware Threat Model
    - user_guide: Add LLVM instructions
    - user_guide: Update prerequisite of TF-A version
    - maintainers: Update maintainers and codeowners

- Framework:
    - fwk_multi_thread: Minor optimisation in MT fwk_thread_put_event
    - fwk sub-element index to identifier getter
    - fwk: Add a check for valid fwk_id type to the framework
    - fwk_thread: Log allocated event instead of submitted event
    - fwk_multi_thread: Log allocated event instead of submitted event

- Modules:
    - bootloader: ensure module config is provided
    - cmn-650: Add support for port aggregation
    - dvfs: Add signal_handler to DVFS
    - isys_rom: idle on failure to boot from ram firmware
    - fip: move fip config parameters to caller module
    - ppu_v1: Assigning opmode from product configurations
    - isys_rom: use fwk_trap() for idling
    - scmi: Deny access for PSCI agent for configured protocols
    - scmi_system_power/boards: Add default timer_id and add extra check
    - scmi: Assorted fixes for SCMI 2.0 compliance

- Platforms:
    - juno: Disable Resource Permissions for juno
    - juno: Disable Debug Module by default
    - juno/sensor: Rename sensors
    - n1sdp: Separated Driver code from mod_n1sdp_sensor.c
    - n1sdp:Enable L3 cache from BOOTGPR1 reg
    - rcar: Various additions for timers, power domain, dvfs, system power
    - sgm775: Enable access to CLOCK_CONFIG_SET command
    - RD-X platforms: migrate to isys_rom
    - sgi575/rdn1e1: Add disabled PSCI protocols without resource permissions
    - sgm775/sgm776/tc0: Add disabled PSCI protocols without resource
    permissions
    - tc0: Add support for Theodul cluster
    - tc0: add clock and dvfs support for all cores
    - product/rdv1mc: add port aggregation related config data

Notes
-----
CMake: please note that cmake is consolidation phase and the current make build
system is still supported. We strongly encourage contributors to move to cmake.

SCP-firmware - version 2.7.0
============================

New features
------------

- Support for the following platforms has been added:
    - Total Compute (tc0) platform
    - Renesas R-Car
    - RD-Daniel Config-XLR

- Support for interactive debugger and CLI has been added.

- New modules:
    - Voltage Domain driver and related SCMIv3 protocol driver
    - gtimer: Add framework time driver implementation
    - module: statistics: add basic functionality
    - fip: Introduce 'fip' parser module

- SCMI Enhancements:
    - SCMI: Resource Permissions Module
    - SCMIv2: Implement SCMIv2 fast channels performance protocol API
    - SCMIv2: Implement notifications for SCMIv2
    - scmi_perf: Add support for performance statistics
    - SCMI: Clock Protocol policy handler
    - SCMI: Performance Protocol policy handler
    - SCMI: Power Domain Protocol policy handler
    - SCMI: System Power Protocol policy handler
    - SCMI: Reset Domain Protocol policy handler
    - SCMI: Base Protocol Permissions
    - SCMI: Clock Protocol Permissions
    - SCMI: System Power Protocol Permissions
    - SCMI: Power Domain Protocol Permissions
    - SCMI: Performance Protocol Permissions
    - SCMI: Sensor Protocol Permissions
    - SCMI: Reset Domain Protocol Permissions
    - SCMI: Device permissions

- Framework:
    - fwk: Use standard library memory allocator
    - fwk: Integrate logging functionality into the framework
    - fwk/thread: Single-threaded mode for blocking events
    - fwk: Introduce input/output component

Changed
-------

- Documentation:
    - doc: SCP Firmware Threat Model
    - doc: Add clang-format configuration
    - doc: Recreate Doxyfile with Doxywizard

- Framework:
    - fwk: Remove dependency on RTX threads
    - fwk: Add support for static element tables
    - fwk: Rework assertion logic
    - fwk: Initialize module context structures early

- Modules:
    - bootloader: Add support for SDS-less boot
    - DVFS: Allow inexact performance levels
    - DVFS/SCMI-perf: Abstract performance levels
    - scmi_system_power: Add graceful system power support
    - bootloader: Unify bootloader messages
    - module/scmi: Add SCMI notifications handling APIs

Notes
-----

This release implements full SCMI v2 support but we do not guarantee complete
compliance with the SCMI v2.0 specification at the moment.

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
