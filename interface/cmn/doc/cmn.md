# CMN Interface
Copyright (c) 2023, Arm Limited. All rights reserved.
## Overview
The CMN interface provides a standard API to be implemented by multiple
modules (CMN drivers).

This will allow multiple modules to have the same interface.
but having different internal implementations.
i.e. It abstracts the knowledge of the API user from the implementation,
    which makes it more flexible and platform-agnostic
## Use
    To use the CMN interface it requires to add the include path in the
    respective module `CMakeLists.txt` file.
    ``` CMAKE
    target_include_directories(${SCP_MODULE_TARGET} PUBLIC
                                      "${CMAKE_SOURCE_DIR}/interface/cmn/")
    ```
    Then simply include `interface_cmn.h` file to use all CMN interface
    definitions.

### Example
    ```C
        /* `api` holds the concrete implementation of the interface. */
       api->map_io_region(uint64_t base, size_t size, uint32_t node_id);
       ```
