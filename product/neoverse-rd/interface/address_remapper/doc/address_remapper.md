# Address Remapper Interface

Copyright (c) 2024, Arm Limited. All rights reserved.

## Overview
The Address remapper interface provides a standard API to be implemented by
modules implementing different techniques of address remapping read/write
support.

This will allow multiple modules to implement the same interface but have
different internal implementations. It abstracts consumers of this interface
from the producers of this interface, which makes it more flexible and
platform-agnostic.

## Use
To use the Address remapper interface,, add the include path of the
interface file in  the respective module `CMakeLists.txt` file.
``` CMAKE
target_include_directories(${SCP_MODULE_TARGET} PUBLIC
      "${CMAKE_SOURCE_DIR}/product/neoverse-rd/interface/address_remapper/")
```
Further, include `interface_address_remapper.h` file to use all address
remapper interface definitions.

### Example
```C
    /* `api` holds the concrete implementation of the interface. */
   api->read8(uint64_t address);
   api->write8(struct address, uint8_t value);
   api->read16(uint64_t address);
   api->write16(struct address, uint16_t value);
   api->read32(uint64_t address);
   api->write32(struct address, uint32_t value);
   api->read64(uint64_t address);
   api->write64(struct address, uint64_t value);
```
