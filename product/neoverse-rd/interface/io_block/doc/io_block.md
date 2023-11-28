# IO Block Interface

Copyright (c) 2024, Arm Limited. All rights reserved.

## Overview
The IO Block interface provides a standard API to be implemented by multiple
modules that implement driver support for the IO block on the Neoverse reference
design platforms.

This will allow multiple modules to implement the same interface but have
different internal implementations. It abstracts consumers of this interface
from the producers of this interface, which makes it more flexible and
platform-agnostic.

## Use
To use the IO Block interface, add the include path of the interface file in the
respective module `CMakeLists.txt` file.
``` CMAKE
target_include_directories(${SCP_MODULE_TARGET} PUBLIC
        "${CMAKE_SOURCE_DIR}/product/neoverse-rd/interface/io_block/")
```
Further, include `interface_io_block.h` file to use all IO Block interface
definitions.

### Example
```C
    /* `api` holds the concrete implementation of the interface. */
    api->map_region(struct interface_io_block_setup_mmap *mmap);
```
