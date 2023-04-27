# Build System

## Overview

The SCP/MCP Software build system is composed of a top-level Makefile and a
collection of .mk files used to build and describe the building blocks and tools
that create the final firmware binaries.

This documentation covers the use of the build system when creating products,
firmware, and modules.

For details on how to build an existing product, please refer to the
documentation using the build system's "help" parameter:

    $> make help

## Product and Firmware Hierarchy

A product is a collection of firmware. All products are located under the
__product__ directory and must adhere to the following hierarchy:

    <root>
     └─ product
        └── <product>
            ├── product.mk
            │   ├── include
            │   │   └── <product level header files...>
            │   └── src
            │       └── <product level source files...>
            ├── <firmware_1>
            │   └── <firmware 1 level configuration files...>
            └── <firmware_2>
                └── <firmware 2 level configuration files...>

Difference products that share similar files can be grouped into
product_group. Shared files are located under __common__ directory, while
all products' specific code are kept under product directory. The following
hierarchy shows the grouped products under __product__ directory:

    <root>
     └─ product
            └──<product_group>
                ├── <common>
                │   │   ├── include
                │   │   │   └── <common header files for products...>
                │   │   └── src
                │   │       └── <common source files for products...>
                └── <product>
                    ├── product.mk
                    │   ├── include
                    │   │   └── <product level header files...>
                    │   └── src
                    │       └── <product level source files...>
                    ├── <firmware_1>
                    │   └── <firmware 1 level configuration files...>
                    └── <firmware_2>
                        └── <firmware 2 level configuration files...>

__Note:__ The names of the \<product\> and \<firmware\> directories must not
contain spaces.

The product.mk is described in the following sections.

### The product.mk File

The product.mk file describes a product to the build system listing all build
images.

The following parameters are mandatory:

* __BS_PRODUCT_NAME__ - Human-friendly name for the product. The content of this
  variable is exposed to the compilation units.
* __BS_PRODUCT_FIRMWARE_LIST__ - List of firmware directories under the current
  product.

# Module

Modules are the building blocks of a product firmware. Modules can be
implemented under the modules/ directory at the root of the project, or they
can be product-specific and implemented under the product/\<product\>/modules
directory. In either case, modules have the following directory structure:

    <module root>
     └── <module>
          ├── include
          │   └── <header files...>
          ├── src
          │    ├── Makefile
          │    └── <source files...>
          ├── lib
          │    └── mod_<module>.a
          ├── test
          │    └── <unit test files...>
          ├── doc
          │    └── <documentation files...>
          ├── CMakeLists.txt
          └── Module.cmake

Only one of the 'src' or 'lib' directories is required. When building a
firmware, if the 'src' directory is present then the module library is built
from the module source code and the 'lib' directory, if present, is ignored.
When only the 'lib' directory is supplied, the module's pre-built static library
is used when building a firmware.

__Note:__ The name of the \<module\> directory must not contain spaces.

The name of the \<module\> directory is used in __SCP_MODULE__ by `cmake`

The __doc__ directory is optional and may contain markdown (.md) based
documentation.

## Module Code Generation

When a firmware is built there are two prerequisite files that will be generated
by the build system, specifically by the __gen_module_code.py__ script:
* fwk_module_idx.h: Contains an enumeration of the indices of the modules that
    make up the firmware. The ordering of the module indices in the enumeration
    within fwk_module_idx.h is guaranteed to follow the order of the module
    names in the BS_FIRMWARE_MODULES list within the firmware's firmware.mk
    file. This same ordering is used by the framework at runtime when performing
    operations that involve iterating over all the modules that are present in
    the firmware, such as the init_modules() function in fwk_module.c.
* fwk_module_list.c: Contains a table of pointers to module descriptors, one
    for each module that is being built as part of the firmware. This file and
    its contents are used internally by the framework and should not normally
    be used by other units such as modules.

# Build Configurations

It is possible to enable different build configurations for each build image,
for more information please refer to [cmake_readme.md](doc/cmake_readme.md).

## Notification Support

When building a firmware and its dependencies, the
`SCP_ENABLE_NOTIFICATIONS` parameter controls whether notification support
is enabled or not.

When notification support is enabled, the following applies:

* The `BUILD_HAS_NOTIFICATION` definition is defined for the units being built.
* Notification specific APIs are made available to the modules via the
  framework components (see [framework.md](doc/framework.md)).

## SCMI Perf Fast Channels Support

When building a firmware and its dependencies, the
`BUILD_HAS_SCMI_PERF_FAST_CHANNELS` parameter controls whether DVFS Fast channel
support is enabled or not.

## SCMI Notifications Support

When building a firmware and its dependencies, the
`BUILD_HAS_SCMI_NOTIFICATIONS` parameter controls whether SCMI notifications
are enabled or not.

## Debug Module Support

When building a firmware and its dependencies, the `BUILD_HAS_DEBUG_UNIT`
parameter controls whether the support for the Debug probe/trace unit is
included.

## SCMI Statistics Support

When building a firmware and its dependencies, the
`BUILD_HAS_STATISTICS` parameter controls whether statistics support is
enabled or not.

## SCMI Agent Resource Permissions

When building a firmware and its dependencies, it is possible to enable a set
of predefined access rules for protocols and agents via the
BS_FIRMWARE_HAS_RESOURCE_PERMISSIONS parameter.

## SCMI Reset Domain Protocol

When building a firmware and its dependencies, the SCMI Reset protocol is
enabled to control of reset-capable domains in the platform. This is possible
via the BS_FIRMWARE_HAS_SCMI_RESET parameter.

## SCMI Sensor Event Notifications

When building a firmware and its dependencies, the
`BUILD_HAS_SCMI_SENSOR_EVENTS` parameter controls if SCMI event notifications
are enabled.

## SCMI Sensor Protocol V2

When building a firmware and its dependencies, the `BUILD_HAS_SCMI_SENSOR_V2`
parameter controls if SCMI Sensor protocol V2 is enabled.

## Clock Tree Management

When building a firmware and its dependencies, the `BUILD_HAS_CLOCK_TREE_MGMT`
parameter controls if Clock Tree Management is enabled.

## Transport based fast channel

When building a firmware and its dependencies, the `BUILD_HAS_MOD_TRANSPORT_FC`
parameter controls if generic transport module implemented fast channel
interface to be used.

## Core Idle Suspend (WFE)

WFE on ARM architecture makes processor suspends it's execution until it
receives any interrupt. SCP firmware will execute this instruction when SCP
firmware is in idle state, that is, when it has finished responding to all the
internal events and external interrupts.

Use `FMW_DISABLE_ARCH_SUSPEND` option to disable this execution of WFE. It
can be defined in a platform specific fmw_arch.h file and adding this file
in product/*/include directory.

## Performance Plugin Handler

/* TODO */

Definitions
===========

The build system sets the following definitions during the compilation of C
and assembly units:

* __BUILD_HOST__ - Set when the CPU target is "host".
* __BUILD_HAS_NOTIFICATION__ - Set when the build has notification support.
* __BUILD_STRING__ - A string containing build information (date, time and git
  commit). The string is assembled using the tool build_string.py.
* __BUILD_TESTS__ - Set when building the framework unit tests.
* __BUILD_MODE_DEBUG__ - Set when building in debug mode.
* __NDEBUG__ - Set when building in release mode. This definition is used by the
  standard C library to disable the assert() support.
* __BUILD_VERSION_MAJOR__ - Major version number.
* __BUILD_VERSION_MINOR__ - Minor version number.
* __BUILD_VERSION_PATCH__ - Patch version number.
* __BUILD_VERSION_STRING__ - String version using the format
  "v<major>.<minor>.<patch>". Example: "v2.3.1".
* __BUILD_VERSION_DESCRIBE_STRING__ - String containing version, date and git
  commit description. If the source code is not under a git repository, the
  string __unknown__ will be used instead.
* __BUILD_HAS_MOD_<MODULE NAME>__ - Set for each module being part of the build.
