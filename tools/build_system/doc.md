Build System
============

Overview
========

The SCP/MCP Software build system is composed of a top-level Makefile and a
collection of .mk files used to build and describe the building blocks and tools
that create the final firmware binaries.

This documentation covers the use of the build system when creating products,
firmware, and modules.

For details on how to build an existing product, please refer to the
documentation using the build system's "help" parameter:

    $> make help

Product and firmware hierarchy
==============================

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
            ├── <firmware>
            │   ├── firmware.mk
            │   └── <firmware level files...>
            └── <firmware>
                ├── firmware.mk
                └── <firmware level files...>

__Note:__ The names of the \<product\> and \<firmware\> directories must not
contain spaces.

The product.mk and firmware.mk files are described in the following sections.

The product.mk file
-------------------

The product.mk file describes a product to the build system.

The following parameters are mandatory:

* __BS_PRODUCT_NAME__ - Human-friendly name for the product. The content of this
  variable is exposed to the compilation units.
* __BS_PRODUCT_FIRMWARE_LIST__ - List of firmware directories under the current
  product.

The firmware.mk file
--------------------

The firmware.mk file describes a firmware to the build system.

The following parameters are mandatory:
* __BS_FIRMWARE_CPU__ - CPU architecture.
* __BS_FIRMWARE_HAS_MULTITHREADING__ <yes|no> - Multithreading support. When set
  to yes, firmware will be built with multithreading support.
* __BS_FIRMWARE_HAS_NOTIFICATION__ <yes|no> - Notification support. When set
  to yes, firmware will be built with notification support.
* __BS_FIRMWARE_NOTIFICATION_COUNT__ < count > - When notification support is set
  to yes, firmware can optinally specifiy the number notfication subscriptions
  to be supported.
* __BS_FIRMWARE_MODULES__ - The list of modules to be included and built into
  the firmware and any APIs to be omitted from each module.
* __BS_FIRMWARE_MODULE_HEADERS_ONLY__ - The list of modules to have their header
  files included into the firmware's build allowing other modules to use their
  definitions. __Note__: These modules are not built into the firmware, only
  their header files are made available.
* __BS_FIRMWARE_SOURCES__ - The list of source files to be built as part of the
  firmware. The source files (.S and .c) can be either at product or firmware
  level.

The format of the __BS_FIRMWARE_MODULES__ parameter can be seen in the following
example:
\code
BS_FIRMWARE_MODULES := module_a \
                       module_b \
                       module_c,foo,bar \
\endcode

In this example, modules A, B, and C will all be built into the
firmware. Module C will have its 'foo' and 'bar' APIs removed.

If a module should remove an API, it will be compiled with a \#define in the
following format:
\code BUILD_DISABLE_API_<api_name_upper> \endcode

For example, module C (only) will have the following definitions when it is
compiled for this firmware:
\code
BUILD_DISABLE_API_FOO
BUILD_DISABLE_API_BAR
\endcode


Module
======

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
          └── doc
               └── <documentation files...>

Only one of the 'src' or 'lib' directories is required. When building a
firmware, if the 'src' directory is present then the module library is built
from the module source code and the 'lib' directory, if present, is ignored.
When only the 'lib' directory is supplied, the module's pre-built static library
is used when building a firmware.

__Note:__ The name of the \<module\> directory must not contain spaces.

The name of the \<module\> directory is used in __BS_FIRMWARE_MODULES__ by the
firmware (see firmware.mk, above).

The __doc__ directory is optional and may contain markdown (.md) based
documentation.

 The following parameters are required in each module's Makefile:
* __BS_LIB_NAME__ - Human-friendly name for the module.
* __BS_LIB_SOURCES__ - List of source files to be built as part of the module.

Module Code Generation
======================

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

Multithreading Support                                 {#section_multithreading}
======================

When building a firmware and its dependencies, the
BS_FIRMWARE_HAS_MULTITHREADING parameter controls whether multithreading support
is enabled or not.

When multithreading support is enabled, the following applies:

* The BUILD_HAS_MULTITHREADING definition is defined for the units being built.
* Multithreading specific APIs are made available to the modules via the
  framework components (see \ref GroupLibFramework).
* The header files from the underlying software layer responsible for providing
  the multithreading capabilities (e.g. embedded OS) are added to the include
  path for the units being built.
* The underlying software responsible for providing the multithreading support
  is included in the firmware during the linking phase.

Notification Support                                     {#section_notification}
====================

When building a firmware and its dependencies, the
BS_FIRMWARE_HAS_NOTIFICATION parameter controls whether notification support
is enabled or not. If notification support is enabled, the firmware can define
the number of notification subscriptions supported on the platform using
BS_FIRMWARE_NOTIFICATION_COUNT parameter.

When notification support is enabled, the following applies:

* The BUILD_HAS_NOTIFICATION definition is defined for the units being built.
* Notification specific APIs are made available to the modules via the
  framework components (see \ref GroupLibFramework).
* The BUILD_NOTIFICATION_COUNT definition is defined if the optional
  parameter BS_FIRMWARE_NOTIFICATION_COUNT is defined. It specified the
  number of notification subscriptions supported and set to the value
  defined by BS_FIRMWARE_NOTIFICATION_COUNT parameter.

Definitions
===========

The build system sets the following definitions during the compilation of C
and assembly units:

* __BUILD_HOST__ - Set when the CPU target is "host".
* __BUILD_HAS_MULTITHREADING__ - Set when the build has multithreading support.
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
