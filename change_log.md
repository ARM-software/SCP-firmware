SCP-firmware Change Log
=======================

Copyright (c) 2019-2024, Arm Limited and Contributors. All rights reserved.

SCP-firmware - version 2.14
============================

New features
------------

- Architecture:
    - arch: add ARM V6M arch
    - arch: add exception fault info print support

- Documentation:
    - doc: add interface documentation

- Framework:
    - framework: Add fastchannel build flag for platforms with DVFS hardware
    - fwk_list: Add a watermark to monitor list max sizes
    - fwk_log: Add option for custom banner
    - assert: Add `FWK_WEAK` to __assert_func()
    - fwk: Add conditional code coverage support for Unit Tests
    - fwk: Add code coverage for framework tests
    - fwk/assert: print location of assertion

- Modules:
    - cmn_cyprus: Add new module
    - scmi_power_capping: Add new module
    - power_coordinator: add mod_power_coordinator.h
    - power_meter: add mod_power_meter.h
    - scmi_power_capping: add resource permissions
    - power_allocator: add mod_power_allocator.h
    - thermal_mgmt: Add derivative term for PI controller
    - amu_mmap: add check for counter offsets while init
    - amu_mmap: add memory-mapped AMU driver
    - sensor_smcf_drv: Add Sensor SMCF data driver
    - interface/cmn: add a generic interface for CMN drivers
    - cdns_i2c: Add bus active check to APIs
    - interface/amu: add documentation for the interface
    - interface/amu: add AMU interface definition
    - optee: Add voltage domain regulators
    - atu: add driver module for Address Translation Unit
    - thermal_mgmt: Add a cold state power configuration
    - smcf: Add MGI validate header for MGI struct definition validation
    - module: Introduce sc_pll driver module
    - module: Add scmi_power_capping
    - scmi_power_capping: add resource permissions
    - scmi_power_capping: Add notifications support
    - scmi_power_capping: Add fast channels support
    - scmi_power_capping: Add PAI and measurements commands
    - power_coordinator: add mod_power_coordinator.h
    - power_meter: add mod_power_meter.h
    - cmn_cyprus: add support for RNSAM node register
    - cmn_cyprus: add support for hierarchical hashing
    - cmn_cyprus: add support for HN-S isolation
    - mod_sensor: Add UT for sensor_api
    - mod_sensor: Add initial UT for fwk handlers
    - timer: add overflow handler API
    - power_domain: additional UTs to the PD
    - ATU: Add if guards when a variable is unused
    - power_domain: additional UTs to the PD
    - power_domain: expanding UTs to cover PD State Checks
    - scmi: add a check for smci message type
    - cmn_cyprus: implement CMN interface API
    - cmn_cyprus: setup RNSAM
    - cmn_cyprus: initialize rnsam table context

- Platform:
    - tc2: Add AMU MMAP driver configuration to TC2
    - product/morello: Add SCMI sensor management
    - morello: Add PCC and MCC firmware version to SDS
    - morello: Add SCP version information to SDS
    - rdfremont: add platform module for mcpfw
    - rdfremont: add timer HAL config data in mcpfw
    - rdfremont: add generic timer config data for mcpfw
    - rdfremont: add config data for MPU module in mcpfw
    - rdfremont: configure I/O stream for mcpfw
    - rdfremont: add config data for pl011 UART module in mcpfw
    - rdfremont: add config data for clock module in mcpfw
    - rdfremont: add build support for scp firmware
    - rdfremont: let modules subscribe to platform notification
    - rdfremont: add config data for scp platform module in scpfw
    - rdfremont: add platform module for scpfw
    - rdfremont: add config data for system power module in scpfw
    - rdfremont: add config data for scmi system power module in scpfw
    - rdfremont: add config data for scmi power domain in scpfw
    - rdfremont: add config data for atu driver in scpfw
    - rdfremont: add config data for scmi module in scpfw
    - rdfremont: add config data for transport module in scpfw
    - rdfremont: add config data for sds module in scpfw
    - rdfremont: add config data for apcontext module in scpfw
    - rdfremont: add config data for mhuv3 driver in scpfw
    - rdfremont: add config data for timer HAL in scpfw
    - rdfremont: add config data for generic timer driver in scpfw
    - rdfremont: add config data for cmn_cyprus driver
    - rdfremont: add config data for clock HAL module in scpfw
    - rdfremont: add config data for ppu v1 module in scpfw
    - rdfremont: add config data for power domain module in scp_ramfw
    - rdfremont: add helpers to obtain platform topology in scp_ramfw
    - rdfremont: add config data for subsystem clock driver in scp_ramfw
    - rdfremont: add config data for PIK clock driver in scp_ramfw
    - rdfremont: add config data for system pll driver in scp_ramfw
    - rdfremont: add System PIK register space declaration
    - rdfremont: add core manager register block declaration
    - rdfremont: add SCP power control block register declaration
    - rdfremont: add MPU module config data for scp_ramfw
    - rdfremont: configure I/O stream for scp_ramfw
    - rdfremont: add pl011 module config data for scp_ramfw
    - rdfremont: add system info module config data for scp_ramfw
    - rdfremont: add sid module config data for scp_ramfw
    - rdfremont: add build support for mcp firmware
    - neoverse-rd: add custom banner for mcpfw
    - tc0: Remove platform code
    - neoverse-rd: define system counter impdef registers

- Unit Test:
    - resource_perms: Add UT for set_agent_resource_<PROTOCOL>_permissions
    - amu_smcf_drv: Add unit test for AMU SMCF data driver module
    - amu_smcf_drv: Add AMU SMCF data driver module
    - mpmm/unit_test: Add unit test for MPMM
    - fch_polled/unit_test: Add unit tests for fch_polled
    - fch_polled: add a new module for fast channels communication
    - amu_mmap: add test for counters_base_addr=null
    - sensor_smcf_drv: Add unit test for sensor_smcf_drv module
    - ppu_v1: add corresponding unit tests
    - mod_scmi_sensor: Add initial UT for fwk handlers
    - scmi_power_capping: Add fast channels unit tests

- Tools:
    - check_build: Add check build to a separate file with YAML integration
    - tools: add utils module banner to all check scripts
    - utils: Add function to filter files from the project
    - utils: create utils module to unify test results
    - check_build: Add check build to a separate file with YAML integration


Changed
-------

- Architecture:
    - Increment nest counter inside critical section
    - threat_model: Change the format and contents

- Build system:
    - check_copyright: Exclude moved files from checks
    - cli: changed reset command to weak symbol to allow override
    - contrib/cmsis: Bump CMSIS version to 5.9.0
    - Makefile: update `MOD_TEST_BUILD_DIR` path
    - cmake: Update Makefile.cmake to list product subdirectories

- Documentation:
    - scp: Update list of maintainers for SCP-firmware
    - scp: Update maintainers references to Gitlab
    - scp: Update repo references to Gitlab
    - doc: Update contribution flow to work with forks
    - user_guide.md: Update user guide

- Framework:
    - fwk: allow override of BUILD_VERSION parameters
    - fwk: Allow building without .git

- Modules:
    - resource_perms: Move backup permissions allocation to init
    - resource perms: Generalising set_agent resource_<protocol>permissions
    - power_domain: Conditionally include power domain notifications
    - power_domain: Move notifications into a seperate file
    - power_domain: Extract state utility functions
    - power_domain: Move contexts into new internal header file
    - cmn700: use interface to define exported apis
    - pcie_integ_ctrl: move the module to the rdn2 product directory
    - transport: allow optional platform notification
    - sds: allow optional platform notification
    - apcontext: allow optional platform notification
    - ppu_v1 Coding style changes
    - ppu_v1 Allow the number of cores per cluster to be configured
    - amu_mmap: AMU interface implementation
    - scmi_perf: Refactor to support fast channel transport
    - fch_polled: Enable complie flag BUILD_HAS_MOD_TRANSPORT_FC
    - transport: Enable compile flag BUILD_HAS_MOD_TRANSPORT_FC
    - module: Minor fch updates to transport and mhu3
    - scmi_system_power_req: Enable multiple elements
    - mod_ppu_v0: Report ppu status
    - mod_ppu_v1: Report ppu status
    - power_domain: Only respond to `SYSTEM_SHUTDOWN` if necessary
    - smcf_utils: Utility for helping with the configuration.
    - sensor_smcf_drv: Align SMCF get_data mock to operate on 32bit buffer
    - mod_transport: Make out-band message support optional
    - mpmm: Update MPMM module to use AMU driver API interface
    - cmn_cyprus: add support for MXP register
    - cmn_cyprus: add support for node info register
    - cmn_cyprus: add support for configuration register
    - cmn_cyprus: add initial framework handlers
    - gtimer: allow impdef registers of system counter to be programmed
    - gtimer: improve bind framework handler
    - mhu3: Include module in module path
    - power_domain: expanding UTs to cover PD State Checks

- Platforms:
    - n1sdp: halt SCP boot if DDR4 training fails
    - morello: halt SCP boot if DDR4 training fails
    - rdn2: move 'rdn2' to product group 'neoverse-rd'
    - rdfremont: move product 'rdfremont' to product group 'neoverse-rd'
    - rdv1mc: move 'rdv1mc' to product group 'neoverse-rd'
    - rdv1: move 'rdv1' to product group 'neoverse-rd'
    - rdn1e1: move 'rdn1e1' to product group 'neoverse-rd'
    - sgi575: move 'sgi575' to product group 'neoverse-rd'
    - rdfremont: Add new configuration
    - morello: print the temperature sensor values during boot
    - rdn2: IONCICLK ctrl/div and TCU/IONCI clock enable should be RW
    - rdn2/scp_ramfw: Exclude SCMI-Perf Ops by default
    - tc2/thermal_mgmt: Use a cold state power configuration
    - Unify TC1 and TC2 folders into Totalcompute
    - n1sdp: Use common sc_pll module
    - morello: Use common sc_pll module
    - rdfremont: configure system counter implementation defined registers
    - rdfremont: reorganize common definitions
    - rdn2: configure system counter implementation defined registers
    - tc2: Reserve memory for RSS's SDS region
    - n1sdp: Use common sc_pll module
    - morello: Use common sc_pll module

- Unit tests:
    - power_domain: add UTs to cover changes to the PD requested state
    - unit_test: List modules in alphabetical order

- Tools:
    - tools/util: removes `text` feature from subprocess
    - check_style: introduce code style check script
    - product: integrate YAML file parsing import
    - check_copyright: modularize and improve readability
    - check_api: modularize and improve readability
    - check_tabs: update script to use file filter from utils
    - check_spacing: modularize and improve readability
    - utils: create utils module to unify test results

Resolved issues
---------------

- Build system:
    - cmake: Remove typo in Makefile.cmake

- Modules:
    - cmn_cyprus: fix incorrect format specifier for Clang
    - cmn_cyprus: suppress release build warnings
    - fch_polled: fix build error when included interface only module
    - power_domain: Address incorrect state update
    - scmi: Shorten long error messages
    - clock: fix mod_pd_restricted_api object identifier
    - scmi_perf: Correct the return status of scmi_perf_process_event
    - sensor_smcf_drv: Fix sensor_smcf_drv unit test
    - amu_smcf_drv: Fix amu_smcf_drv unit test
    - amu_mmap: fix memory allocation for core counters
    - cmn700: fix address and size alignment check
    - pcie_integ_ctrl: fix codestyle format
    - mhu3: Correct the conditional check for MHU in/out channel
    - pcid: fix incorrect check of PID/CID registers
    - mod_ppu_v0: Fix error message
    - mod_ppu_v1: Fix error message
    - SMCF: Fix enablement of START and END sample identifiers in WRCFG
    - cmn_cyprus: setup HNFSAM
    - cmn_cyprus: initialize hns table context
    - cmn_cyprus: support for HN-S node register
    - cmn_cyprus: setup CMN on clock notification
    - cmn_cyprus: enable CMN discovery
    - cmn_cyprus: fix incorrect format specifier for clang
    - cmn_cyprus: suppress release build warnings
    - optee/clock: fix indentation
    - optee/clock: fix device error cases

- Platforms:
    - tc1/ram_fw: Fix inclusion of resource permissions in target
    - tc2/ram_fw: Fix inclusion of resource permissions in target
    - morello: supress Cppcheck error on macro substitution
    - rdn2: remove incorrect configuration of system counter increment value
    - rdn2: fix refclk clock speed

- Unit tests:
    - unit_test: Fix order of list modules
    - unit_test: add variable initialization
    - scmi_power_capping: Fix unit tests

- Version Control:
    - Add a GitHub Actions to auto comment PR

- Tools:
    - check_copyright: fix default commit hash id

SCP-firmware - version 2.13
============================

New features
------------

- Modules:
    - System Monitoring Control Framework: It is a module designed to manage
      a large and diverse set of on-chip sensors and monitors, and
      corresponding unit tests


Changed
-------

- Documentation:
    - maintainers: Update maintainers list
    - doc/framework: Update with structs for static tables

- Build system:
    - cmake: Add target info for fwk_test and mod_test
    - ci_cmake: Warn users when script runs without docker support

- Framework:
    - fwk_io: Extended functionality with the introduction of
      fwk_io_putch_nowait and add FWK_E_BUSY return to putch
      when the resource is busy
    - Add disable option to log level
    - fwk_log: Remove '\n' character from log prints
    - fwk_io: Replace fwk_assert for error handling

- Modules:
    - scmi_perf: Improvements in code separation for SCMI-Perf
    - system_power: A change to allow zero elements for systems without PPUs
    - scmi: Introduction of a notification handler
    - scmi_system_power: Refactoring of the set_state function
    - optee: Unit test framework
    - dvfs: Initial unit tests
    - power_domain: Update all power state variables upon report
    - scmi_clock: Improve Unit tests
    - pl011: Enhanced pl011 driver with simplification
    - pl011: Initial unit tests
    - cmn700: Using offset adjusted address for remote chips
    - cmn700: Allow runtime addition of io region in rnsam, alongside enabling
      and disbaling rnsam and refactoring.
    - xr77128: Add xr77128 (PMIC) module
    - scmi_perf: Improvements and unit testing for the Plugins Handler
      extension in SCMI-Performance

- Platforms:
    - synquacer: Prevent metadata from being updated on every boot
    - rdn2: Switch ON SYSTOP PPU by default at boot to allow other modules
      early initialization
    - juno: Introduce platform variant to decrease the binary size
    - n1sdp: Remove exception_handler function
    - morello: Enable voltage scaling in DVFS framework
    - n1sdp: Add configs for timer and gtimer in MCP RAM firmware and port to
      generic modules for MCP-SCP SCMI comms and fix these
    - morello: Add configs for timer and gtimer in MCP RAM firmware
      and port to generic modules for SCP-MCP SCMI comms and fix these
    - morello: enable automatic sampling and temperature interrupts
    - juno: The platform variants added for CI
    - all platforms: Fix delayed transport init
    - all platforms: Allow platforms to specify valid PID/CID registers

- Unit tests:
    - user_guide: Clarify requirements for contributors
    - Add template for Quick Start

Resolved issues
---------------
- Documentation:
    - user_guide: Amendments to the user guide
    - Doxygen: Amend warnings and dependencies

- Build system:
    - docker: fix missing dependency for LLVM compiler

- Framework:
    - fwk/Makefile: Fix missing references for FWK_LOG_LEVEL_TRACE
    - Fix dlist_remove/slist_next util functions declaration

- Modules:
    - mhu3: Fix bind function
    - msys_rom: Correction for building with log level disabled
    - pl011: Fix to address incorrect assert when fractional divisor is zero

- Platforms:
    - tc2: Correction for building with log level disabled
    - n1sdp: Correction for building with CRIT
    - n1sdp: Remove unused multichip info structure
    - morello: Remove unused multichip info struct
    - morello: Fix build when log level set to DISABLED or CRIT
    - rdn2: Fixed boot issue


SCP-firmware - version 2.12
============================

New features
------------

- Documentation:
    - security.md file: Add the details of the SCP FW security process

- Framework:
    - FWK_TRACE: Allows printing of per-module enabled logs

- Modules:
    - SCMI System Power requestor: It is a module that takes an SCMI SYSTEM
      POWER command and forwards it to its element, in such a way that it is
      received by them as if it were purely a System Power command.

- Platforms:
    - OPTEE FVP platform product
    - stm32mp1 OP-TEE platform product

Changed
-------

- Documentation:
    - maintainers: Update maintainers list

- Build system:
    - Remove all Makefile deprecated files
    - cmake: Add a command for merging SCP libraries
    - cmake: add option to replace modules
    - Remove Makefile references in documentation

- Framework:
    - Rename `FWK_LOG_LEVEL_TRACE` to `FWK_LOG_LEVEL_DEBUG`

- Modules:
    - power_domain: Add warm reset notifications
    - Use transport module instead of smt and remove smt
    - platform_system: Add warm reset support
    - scmi_perf: Add logs when multiple fast channels events are queued
    - scmi_perf: Add initial UT for plugins-handler extension
    - thermal_mgmt: Add UT and activity factor

- Platforms:
    - Rename RD-Edmunds to RD-V2
    - All platforms: Move from module smt to transport
    - morello: Configure the default OPP to the highest
    - rdfremont/lcp: Append bl2 to firmware target
    - product/rdn2: Add support for variant 3 of rdn2 platform3
    - platform/rdn2: Enable soc expansion block on RD-N2-Cfg2
    - tc2: Add support for TC2_VAR_EXPERIMENT_POWER
    - product/tc2: Add the core HUNTER_ELP

Resolved issues
---------------

- Build system:
    - cmake: fix IPO dependant option
    - ci_cmake: Add missing version 3 for RD-N2 build

- Framework:
    - fwk_core/fwk_arch: Fix buffered log output behaviour

- Modules:
    - cmn600: add missing revision r3p2
    - cmn600: fix print of RN-F and RN-D counts
    - power_domain: Fix return status for notify_warm_reset
    - system_power: respond with correct status for shutdown/reboot
    - module_common: fix FWK_SRC scoping
    - scmi_clock: Fix policy reference counter mismatch

- Platforms:
    - synquacer: allocate secure DRAM for BL31

Deprecated
----------

- Platforms:
    - tc0: Deprecate platform

Known issues
------------

- RDN2 fails to boot. Refer to the following github issue for more details and
  the available temporary workaround:
  https://github.com/ARM-software/SCP-firmware/issues/781

SCP-firmware - version 2.11
============================

New features
------------

- Architecture:
    - ARMv8-M support and V8 MPU support

- New modules:
    - SCMI requester core module support
    - SCMI Sensor requester module
    - Transport module
    - MHUv3 support

- Platforms:
    - TC2 platform support
    - rdfremont: Add new plaform (initial support)

- Tools:
    - Update CMSIS to newer version 5.8.0
    - Unit Test framework and Mocking

Changed
-------

- Build system:
    - ci: Print build output/information
    - cmake: update LLVM to ver 13

- Framework:
    - fwk: Add support of OS interrupt operation
    - fwk: Add stop sequence

- Platforms:
    - n1sdp: Introduce trusted board boot
    - n1sdp: Replace sensor library with a dummy source file
    - product/rdn2: add support for rdn2 platform variant 2
    - product/rdn2: Update configuration data for SCMI perf
    - product/rdv1mc: Update configuration data for SCMI perf
    - product/rdv1: Update configuration data for SCMI perf
    - rdn2: Add variant 1 and 2
    - product/rdn2: enable sp805 watchdog module
    - product/tc2: Add support for TC2/RSS boot flow

- Modules:
    - scmi: separate scmi base protocol
    - fip: extend fip module to accept custom uuid through module config
    - scmi-perf: move fast channel alarm to event context
    - thermal-mgmt: Move power allocation to a separate file
    - sensor: typedef included to give signed or unsigned value options
    - power-mgmt: add per-domain control loop
    - thermal-mgmt: add temperature protection functionality
    - SCMI: Rename BUILD_HAS_FAST_CHANNELS with _SCMI_PERF_
    - scmi: add unit tests
    - scmi_clock: add unit test
    - mhu2: add support for transport module and in-band messages
    - scmi_perf: Add initial unit tests

Resolved issues
---------------

- Build system:
    - ci_cmake: Add flag to skip container execution
    - cmake: Fix ArmClang generated images
    - ci: fix pycodestyle violations in ci_cmake.py
    - CMake: Add elf extension to firmware target

- Modules:
    - modules: Compile without notifications
    - scmi_clock: fix set_rate flag mask defect
    - cmn700: fix SYS_CACHE_GRP_SN_NODEID calculation
    - cmn700: ccg: various fixes

- Platforms:
    - morello: fix timestamp in debug logs
    - product/morello: align DVFS frequency values of FVP with SoC
    - juno: Fix the number of rates returns for triplet rate array format

- CLI:
    - cli: stop alarm when cli is open

Remarks
-------

The codebase attempts to follow MISRA-2012 rules to the extent detailed in
doc/code_rules.md.

The maintainers are gradually introducing tools in the attempt to improve the
overall robustness of the codebase. Since v.2.10, Unit Testing has been
introduced on a module-level. The final aim is to run unit tests for any new
modules and any new additions to modules. To allow contributors to some time to
familiarise with the tool and to avoid imposing additional efforts
out-of-the-blue, in this release we ask contributors to provide basic unit tests
for new modules and new additions. This can be done by adding unit tests on the
same patches adding new features or in separate patches, but need to be within
the same PR. For work that is already in the RFC in PR, we may make an exception
to the request.
We foresee that once the unit test framework support is consolidated, it will be
possible to contribute to the project only if unit test is also included in the
contributions.

SCP-firmware - version 2.10
============================

New features
------------

- Build system:
    - Stable CMake support has been added
    - Make build system is being deprecated in this release, and in a future
      release will be removed completely. No further patches related to the old
      make build system will be accepted.
    - LLVM support for armv7-m platforms

- New modules:
    - Max Power Mitigation Mechanism (MPMM) (initial support)
    - Traffic Cop performance plugin (initial support)
    - Added Thermal management (initial support)

- Platforms:
    - Morello platform support

- Framework:
    - Removed multi-threading feature, all platforms are now single threaded
      There is no longer any thread concept, please see relevant commits for
      details.
    - framework: Add fwk_string interface

Changed
-------

Arm is committed to making the language we use inclusive, meaningful, and
respectful. Offensive terminology has been replaced with more inclusive
terms throughout the codebase.

The codebase now follows MISRA-2012 rules to the extent detailed in
doc/code_rules.md.

- Build system:
    - Add target "doc" to cmake build system

- Framework:
    - fwk: Remove deprecated function put_event_and_wait
    - fwk: Reduce SCP power consumption by suspending execution

- Platforms:
    - tc0: Add cmake support for plugins handler
    - tc0: Add Power Model
    - tc0: Add support for Build Variants
    - tc0: Add PLATFORM_VARIANT feature flag
    - sgi575/rdn1e1/rdv1/rdv1mc: add cmn650 mapping for 64-bit pcie mmio address space
    - rdn1e1/rdv1/rdv1mc/rdn2/sgi575: fix gtimer module config in mcp ramfw
    - synquacer: implement system power-off
    - rdn2: add mapping for cmn700 address space

- Modules:
    - scmi_perf: clean up scmi_perf_notify_limits_updated()
    - scmi_pd: Add power domain set state sync request handling
    - scmi_sensor/sensor: Multi axis support
`   - scmi_sensor/sensor: add timestamp support
    - scmi_sensor/sensor: Extended Attributes Support
    - perf_plugins_handler: Allow multiple plugins
    - perf_plugins_handler: Add support for full data snapshot
    - gtimer: add new flag for CNTCONTROL register initialization

Resolved issues
---------------

- Build system:
    - Fix SCP_ENABLE_DEBUGGER feature flag

- Modules:
    - scmi: shrink log strings to fix error messages
    - plugins_handler: fix cmake build

- Platforms:
    - tc0: Fix Voltage Domain configuration
    - synquacer: enable DDR DQS gate training workaround
    - synquacer: fix spi nor flash software reset handling

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
    - scmi: Add SCMI notifications handling APIs

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
